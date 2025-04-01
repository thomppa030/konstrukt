#include "VulkanContext.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "core/CoreTypes.hpp"
#include "core/log/Logger.hpp"

// Include platform-specific Vulkan headers
#ifdef _WIN32
#  define VK_USE_PLATFORM_WIN32_KHR
#  define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__linux__)
#  define VK_USE_PLATFORM_XLIB_KHR
#  define VK_USE_PLATFORM_WAYLAND_KHR
#  define GLFW_EXPOSE_NATIVE_X11
#  define GLFW_EXPOSE_NATIVE_WAYLAND
#elif defined(__APPLE__)
#  define VK_USE_PLATFORM_MACOS_MVK
#  define GLFW_EXPOSE_NATIVE_COCOA
#endif

// Include GLFW with native platform support
#include <GLFW/glfw3.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3native.h>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_wayland.h>
#include <vulkan/vulkan_xlib.h>

#include "renderer/core/GraphicsDevice.hpp"

namespace kst::renderer::core {

  static VKAPI_ATTR auto VKAPI_CALL debugCallback(
      VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
      VkDebugUtilsMessageTypeFlagsEXT messageType,
      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
      void* /*pUserData*/
  ) -> VkBool32 {
    // Get message type as string for better logging
    std::string typeStr;
    if ((messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) != 0U) {
      typeStr = "GENERAL";
    } else if ((messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) != 0U) {
      typeStr = "VALIDATION";
    } else if ((messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) != 0U) {
      typeStr = "PERFORMANCE";
    } else {
      typeStr = "UNKNOWN";
    }

    // Format message with additional information from callback data
    std::string message = "Vulkan [" + typeStr + "]: " + pCallbackData->pMessage;

    // Add any object information if available
    if (pCallbackData->objectCount > 0) {
      message += "\nObjects:";
      for (uint32_t i = 0; i < pCallbackData->objectCount; ++i) {
        const auto& obj = pCallbackData->pObjects[i];
        message += "\n  - Type: " + std::to_string(obj.objectType);
        if (obj.pObjectName != nullptr) {
          message += ", Name: " + std::string(obj.pObjectName);
        }
      }
    }

    // Log to appropriate level based on severity
    if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0) {
      kst::core::Logger::error<>(message);
    } else if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) != 0) {
      kst::core::Logger::warn<>(message);
    } else if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) != 0) {
      kst::core::Logger::info<>(message);
    } else {
      kst::core::Logger::debug<>(message);
    }

    // Return false to indicate the error shouldn't terminate
    return VK_FALSE;
  }

  VulkanContext::VulkanContext() {}

  VulkanContext::~VulkanContext() {
    try {
      kst::core::Logger::debug("VulkanContext destructor called");
      shutdown();
    } catch (const std::exception& e) {
      kst::core::Logger::error<const char*>("Error in VulkanContext destructor: {}", e.what());
    } catch (...) {
      kst::core::Logger::error("Unknown error in VulkanContext destructor");
    }
  }

  auto VulkanContext::initialize(void* windowHandle, uint32_t width, uint32_t height) -> bool {
    try {
      if (!createInstance()) {
        return false;
      }

      if (!setupDebugMessenger()) {
        return false;
      }

      if (!createSurface(windowHandle)) {
        return false;
      }

      if (!pickPhysicalDevice()) {
        return false;
      }

      if (!createLogicalDevice()) {
        return false;
      }

      m_vulkanDevice.initialize(m_physicalDevice);

      if (!createSwapchain(width, height)) {
        return false;
      }

      if (!createImageViews()) {
        return false;
      }

      if (!createCommandPool()) {
        return false;
      }

      if (!createCommandBuffers()) {
        return false;
      }

      return createSyncObjects();

    } catch (const std::exception& e) {
      kst::core::Logger::error<const char*>("Error initializing Vulkan: {}", e.what());
      shutdown();
      return false;
    }
  }

  void VulkanContext::shutdown() {
    kst::core::Logger::debug("VulkanContext::shutdown() called");

    try {
      // First wait for device to be idle if it's valid
      if (m_device != VK_NULL_HANDLE) {
        kst::core::Logger::debug("Waiting for device to be idle");
        vkDeviceWaitIdle(m_device);
      }

      // Clear all the synchronization objects and command objects first
      // and ensure we have a valid device before destroying them
      if (m_device != VK_NULL_HANDLE) {
        // Clean up synchronization objects
        kst::core::Logger::debug<size_t>(
            "Cleaning up {} semaphores and fences",
            m_imageAvailalableSemaphores.size()
        );

        // Store current device for safety
        VkDevice deviceToDestroy = m_device;

        // Destroy semaphores and fences
        for (size_t i = 0; i < m_imageAvailalableSemaphores.size(); i++) {
          if (m_imageAvailalableSemaphores[i] != VK_NULL_HANDLE) {
            vkDestroySemaphore(deviceToDestroy, m_imageAvailalableSemaphores[i], nullptr);
            m_imageAvailalableSemaphores[i] = VK_NULL_HANDLE;
          }

          if (i < m_renderFinishedSemaphores.size() &&
              m_renderFinishedSemaphores[i] != VK_NULL_HANDLE) {
            vkDestroySemaphore(deviceToDestroy, m_renderFinishedSemaphores[i], nullptr);
            m_renderFinishedSemaphores[i] = VK_NULL_HANDLE;
          }

          if (i < m_inFlightFences.size() && m_inFlightFences[i] != VK_NULL_HANDLE) {
            vkDestroyFence(deviceToDestroy, m_inFlightFences[i], nullptr);
            m_inFlightFences[i] = VK_NULL_HANDLE;
          }
        }

        // Clear vectors
        m_imageAvailalableSemaphores.clear();
        m_renderFinishedSemaphores.clear();
        m_inFlightFences.clear();

        // Clear command pool and buffers
        kst::core::Logger::debug("Cleaning up command pool");
        if (m_commandPool != VK_NULL_HANDLE) {
          vkDestroyCommandPool(deviceToDestroy, m_commandPool, nullptr);
          m_commandPool = VK_NULL_HANDLE;
        }
        m_commandBuffers.clear();

        // Cleanup swapchain before destroying device
        kst::core::Logger::debug("Cleaning up swapchain resources");
        cleanupSwapchain();

        // Finally destroy the device
        kst::core::Logger::debug("Destroying logical device");
        vkDestroyDevice(deviceToDestroy, nullptr);
        m_device = VK_NULL_HANDLE;
      }
    } catch (const std::exception& e) {
      kst::core::Logger::error<const char*>("Error cleaning up device resources: {}", e.what());
    } catch (...) {
      kst::core::Logger::error("Unknown error cleaning up device resources");
    }

    try {
      // Clean up remaining Vulkan resources
      if (m_surface != VK_NULL_HANDLE && m_instance != VK_NULL_HANDLE) {
        kst::core::Logger::debug("Destroying surface");
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
      }

      if (m_debugMessenger != VK_NULL_HANDLE && m_instance != VK_NULL_HANDLE) {
        kst::core::Logger::debug("Destroying debug messenger");
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT
        )vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");

        if (func != nullptr) {
          func(m_instance, m_debugMessenger, nullptr);
        }
        m_debugMessenger = VK_NULL_HANDLE;
      }

      if (m_instance != VK_NULL_HANDLE) {
        kst::core::Logger::debug("Destroying Vulkan instance");
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
      }

      kst::core::Logger::debug("VulkanContext shutdown completed successfully");
    } catch (const std::exception& e) {
      kst::core::Logger::error<const char*>("Error cleaning up instance resources: {}", e.what());
    } catch (...) {
      kst::core::Logger::error("Unknown error cleaning up instance resources");
    }
  }

  auto VulkanContext::getDevice() -> GraphicsDevice& {
    return m_vulkanDevice;
  }

  auto VulkanContext::convertResourceStateToAccessFlags(::kst::core::ResourceState state)
      -> VkAccessFlags {
    switch (state) {
    case ::kst::core::ResourceState::UNDEFINED:
      return 0;

    case ::kst::core::ResourceState::GENERAL:
      return VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;

    case ::kst::core::ResourceState::VERTEX_BUFFER:
      return VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;

    case ::kst::core::ResourceState::INDEX_BUFFER:
      return VK_ACCESS_INDEX_READ_BIT;

    case ::kst::core::ResourceState::CONSTANT_BUFFER:
      return VK_ACCESS_UNIFORM_READ_BIT;

    case ::kst::core::ResourceState::INDIRECT_BUFFER:
      return VK_ACCESS_INDIRECT_COMMAND_READ_BIT;

    case ::kst::core::ResourceState::SHADER_RESOURCE:
      return VK_ACCESS_SHADER_READ_BIT;

    case ::kst::core::ResourceState::UNORDERED_ACCESS:
      return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

    case ::kst::core::ResourceState::RENDER_TARGET:
      return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    case ::kst::core::ResourceState::DEPTH_STENCIL_READ:
      return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

    case ::kst::core::ResourceState::DEPTH_STENCIL_WRITE:
      return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    case ::kst::core::ResourceState::SHADER_READ:
      return VK_ACCESS_SHADER_READ_BIT;

    case ::kst::core::ResourceState::SHADER_WRITE:
      return VK_ACCESS_SHADER_WRITE_BIT;

    case ::kst::core::ResourceState::COPY_SOURCE:
      return VK_ACCESS_TRANSFER_READ_BIT;

    case ::kst::core::ResourceState::COPY_DESTINATION:
      return VK_ACCESS_TRANSFER_WRITE_BIT;

    case ::kst::core::ResourceState::PRESENT:
      return 0;

    default:
      kst::core::Logger::warn<int>("Unknown resource state: {}", static_cast<int>(state));
      return 0;
    }
  }

  auto VulkanContext::convertResourceStateToPipelineStage(::kst::core::ResourceState state)
      -> VkPipelineStageFlags {
    switch (state) {
    case ::kst::core::ResourceState::UNDEFINED:
      return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    case ::kst::core::ResourceState::GENERAL:
      return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    case ::kst::core::ResourceState::VERTEX_BUFFER:
      return VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;

    case ::kst::core::ResourceState::INDEX_BUFFER:
      return VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;

    case ::kst::core::ResourceState::CONSTANT_BUFFER:
      return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
             VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

    case ::kst::core::ResourceState::INDIRECT_BUFFER:
      return VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;

    case ::kst::core::ResourceState::SHADER_RESOURCE:
    case ::kst::core::ResourceState::SHADER_READ:
      return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
             VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

    case ::kst::core::ResourceState::UNORDERED_ACCESS:
    case ::kst::core::ResourceState::SHADER_WRITE:
      return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
             VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

    case ::kst::core::ResourceState::RENDER_TARGET:
      return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    case ::kst::core::ResourceState::DEPTH_STENCIL_READ:
    case ::kst::core::ResourceState::DEPTH_STENCIL_WRITE:
      return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

    case ::kst::core::ResourceState::COPY_SOURCE:
    case ::kst::core::ResourceState::COPY_DESTINATION:
      return VK_PIPELINE_STAGE_TRANSFER_BIT;

    case ::kst::core::ResourceState::PRESENT:
      return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

    default:
      kst::core::Logger::warn<int>("Unknown resource state: {}", static_cast<int>(state));
      return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }
  }

  auto VulkanContext::convertResourceStateToImageLayout(::kst::core::ResourceState state)
      -> VkImageLayout {
    switch (state) {
    case ::kst::core::ResourceState::UNDEFINED:
      return VK_IMAGE_LAYOUT_UNDEFINED;

    case ::kst::core::ResourceState::GENERAL:
      return VK_IMAGE_LAYOUT_GENERAL;

    case ::kst::core::ResourceState::SHADER_RESOURCE:
    case ::kst::core::ResourceState::SHADER_READ:
      return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    case ::kst::core::ResourceState::UNORDERED_ACCESS:
    case ::kst::core::ResourceState::SHADER_WRITE:
      return VK_IMAGE_LAYOUT_GENERAL;

    case ::kst::core::ResourceState::RENDER_TARGET:
      return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    case ::kst::core::ResourceState::DEPTH_STENCIL_READ:
      return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    case ::kst::core::ResourceState::DEPTH_STENCIL_WRITE:
      return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    case ::kst::core::ResourceState::COPY_SOURCE:
      return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    case ::kst::core::ResourceState::COPY_DESTINATION:
      return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    case ::kst::core::ResourceState::PRESENT:
      return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    default:
      kst::core::Logger::warn<int>(
          "Unknown resource state for image layout: {}",
          static_cast<int>(state)
      );
      return VK_IMAGE_LAYOUT_UNDEFINED;
    }
  }

  auto VulkanContext::createInstance() -> bool {
    bool enableValidationLayers = false;

#ifdef _DEBUG
    enableValidationLayers = true;
#endif

    std::vector<const char*> validationLayers;
    if (enableValidationLayers) {
      validationLayers.push_back("VK_LAYER_KHRONOS_validation");

      uint32_t layerCount = 0;
      vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
      std::vector<VkLayerProperties> availableLayers(layerCount);
      vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

      for (const auto& layer : availableLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
          if (strcmp(layer.layerName, layerProperties.layerName) == 0) {
            layerFound = true;
            break;
          }
        }

        if (!layerFound) {
          kst::core::Logger::warn<const char*>(
              "Validation layer {} not available",
              layer.layerName
          );
          enableValidationLayers = false;
          validationLayers.clear();
          break;
        }
      }
    }

    VkApplicationInfo appInfo  = {};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "Konstrukt Renderer";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "Konstrukt";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_4;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo     = &appInfo;

    std::vector<const char*> extensions;

    // Get required extensions from GLFW
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    extensions.reserve(glfwExtensionCount);
    for (uint32_t i = 0; i < glfwExtensionCount; i++) {
      extensions.push_back(glfwExtensions[i]);
    }

    // Add additional platform-specific extensions
#ifdef _WIN32
    extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__linux__)
    extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
    extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(__APPLE__)
    extensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#endif

    if (enableValidationLayers) {
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    createInfo.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    createInfo.enabledLayerCount   = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();

    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
    if (result != VK_SUCCESS) {
      kst::core::Logger::error("Failed to create Vulkan instance");
      return false;
    }

    return true;
  }

  auto VulkanContext::setupDebugMessenger() -> bool {
#ifndef _DEBUG
    return true;
#endif

    // Setup for debug messenger
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;

    // Get function pointer and create debug messenger
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT
    )vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");

    if (func != nullptr) {
      VkResult result = func(m_instance, &createInfo, nullptr, &m_debugMessenger);
      if (result != VK_SUCCESS) {
        kst::core::Logger::error("Failed to set up debug messenger");
        return false;
      }
    } else {
      kst::core::Logger::error<>("Could not find vkCreateDebugUtilsMessengerEXT function");
      return false;
    }

    return true;
  }

  auto VulkanContext::createSurface(void* windowHandle) -> bool {
    if (windowHandle == nullptr) {
      kst::core::Logger::error("Null window handle in createSurface");
      return false;
    }

    if (m_instance == VK_NULL_HANDLE) {
      kst::core::Logger::error("Null Vulkan instance in createSurface");
      return false;
    }

    auto* window = reinterpret_cast<GLFWwindow*>(windowHandle);

    VkResult result = glfwCreateWindowSurface(m_instance, window, nullptr, &m_surface);
    if (result != VK_SUCCESS) {
      kst::core::Logger::error("Failed to create Vulkan surface using GLFW");
      return false;
    }
    kst::core::Logger::info<>("Created Vulkan surface using GLFW");

    return true;
  }

  struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    auto isComplete() const -> bool {
      return graphicsFamily.has_value() && presentFamily.has_value();
    }
  };

  auto findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) -> QueueFamilyIndices {
    QueueFamilyIndices indices;

    // Safety check - don't proceed with null handles
    if (device == VK_NULL_HANDLE || surface == VK_NULL_HANDLE) {
      kst::core::Logger::error<>("Null device or surface handle in findQueueFamilies");
      return indices;
    }

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    // Find queue families that support graphics and presentation
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
      // Check for graphics support
      if ((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0U) {
        indices.graphicsFamily = i;
      }

      // Check for presentation support
      VkBool32 presentSupport = VK_FALSE;

      // Safe call to vkGetPhysicalDeviceSurfaceSupportKHR
      VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
      if (result == VK_SUCCESS && presentSupport == VK_TRUE) {
        indices.presentFamily = i;
      }

      // If we found both, we can exit early
      if (indices.isComplete()) {
        break;
      }
    }

    return indices;
  }

  auto checkDeviceExtensionSupport(VkPhysicalDevice device) -> bool {
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(
        device,
        nullptr,
        &extensionCount,
        availableExtensions.data()
    );

    // Required extensions
    std::set<std::string> requiredExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    // Check if all required extensions are available
    for (const auto& extension : availableExtensions) {
      requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
  }

  auto isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) -> int {
    // Safety check
    if (device == VK_NULL_HANDLE || surface == VK_NULL_HANDLE) {
      kst::core::Logger::error<>("Null device or surface handle in isDeviceSuitable");
      return 0; // Not suitable
    }

    // Get device properties and features
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    // Check if queue families supported
    QueueFamilyIndices indices = findQueueFamilies(device, surface);
    if (!indices.isComplete()) {
      kst::core::Logger::debug<const char*>(
          "Device {} does not have complete queue families",
          deviceProperties.deviceName
      );
      return 0; // Not suitable
    }

    // Check if required extensions are supported
    bool extensionsSupported = checkDeviceExtensionSupport(device);
    if (!extensionsSupported) {
      kst::core::Logger::debug<const char*>(
          "Device {} does not support required extensions",
          deviceProperties.deviceName
      );
      return 0; // Not suitable
    }

    // Assign a score based on device properties
    int score = 1; // Base score for a suitable device

    // Prefer discrete GPUs
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      score += 1000;
    } else if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
      score += 500;
    }

    // Prefer devices with more memory
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(device, &memProperties);

    VkDeviceSize totalMemory = 0;
    for (uint32_t i = 0; i < memProperties.memoryHeapCount; i++) {
      if ((memProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0U) {
        totalMemory += memProperties.memoryHeaps[i].size;
      }
    }

    // Add score based on memory (1 point per GB)
    score += static_cast<int>(totalMemory / (1024 * 1024 * 1024));

    // Check for geometry shader support if needed
    if (deviceFeatures.geometryShader != 0U) {
      score += 100;
    }

    // Check for tessellation shader support if needed
    if (deviceFeatures.tessellationShader != 0U) {
      score += 100;
    }

    kst::core::Logger::debug<const char*, int, int>(
        "Device: {}, Type: {}, Score: {}",
        deviceProperties.deviceName,
        deviceProperties.deviceType,
        score
    );

    return score;
  }

  auto VulkanContext::pickPhysicalDevice() -> bool {
    // Check for valid instance and surface
    if (m_instance == VK_NULL_HANDLE) {
      kst::core::Logger::error("Null Vulkan instance in pickPhysicalDevice");
      return false;
    }

    if (m_surface == VK_NULL_HANDLE) {
      kst::core::Logger::error("Null Vulkan surface in pickPhysicalDevice");
      return false;
    }

    // Get available physical devices
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
      kst::core::Logger::error("Failed to find GPUs with Vulkan support");
      return false;
    }

    kst::core::Logger::info<uint32_t>("Found {} Vulkan-compatible device(s)", deviceCount);

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

    // Rate devices and pick the best one
    std::multimap<int, VkPhysicalDevice> candidates;

    for (const auto& device : devices) {
      int score = isDeviceSuitable(device, m_surface);
      if (score > 0) {
        candidates.insert(std::make_pair(score, device));
      }
    }

    // Check if we found any suitable device
    if (candidates.empty()) {
      kst::core::Logger::error("Failed to find a suitable GPU");
      return false;
    }

    // Choose the device with the highest score
    m_physicalDevice = candidates.rbegin()->second;

    // Get device properties for logging
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(m_physicalDevice, &deviceProperties);

    // Log the selected device
    kst::core::Logger::info<const char*>(
        "Selected physical device: {}",
        deviceProperties.deviceName
    );

    // Log detailed device information
    std::string deviceType;
    switch (deviceProperties.deviceType) {
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
      deviceType = "Integrated GPU";
      break;
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
      deviceType = "Discrete GPU";
      break;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
      deviceType = "Virtual GPU";
      break;
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
      deviceType = "CPU";
      break;
    default:
      deviceType = "Unknown";
      break;
    }

    kst::core::Logger::info<std::string>("Device Type: {}", deviceType);
    kst::core::Logger::info<uint32_t, uint32_t, uint32_t>(
        "API Version: {}.{}.{}",
        VK_VERSION_MAJOR(deviceProperties.apiVersion),
        VK_VERSION_MINOR(deviceProperties.apiVersion),
        VK_VERSION_PATCH(deviceProperties.apiVersion)
    );

    // Store the queue family indices for later use
    [[maybe_unused]] QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice, m_surface);

    return true;
  }

  auto VulkanContext::createLogicalDevice() -> bool {
    // Get queue family indices
    QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice, m_surface);

    // Create a set of unique queue families
    std::set<uint32_t> uniqueQueueFamilies = {
        indices.graphicsFamily.value(),
        indices.presentFamily.value()
    };

    // Create queues for each queue family
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies) {
      VkDeviceQueueCreateInfo queueCreateInfo{};
      queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = queueFamily;
      queueCreateInfo.queueCount       = 1;
      queueCreateInfo.pQueuePriorities = &queuePriority;
      queueCreateInfos.push_back(queueCreateInfo);
    }

    // Specify device features
    VkPhysicalDeviceFeatures deviceFeatures{};

    // Enable required features
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    // Specify device extensions
    std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    // Create logical device
    VkDeviceCreateInfo createInfo{};
    createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos       = queueCreateInfos.data();
    createInfo.pEnabledFeatures        = &deviceFeatures;
    createInfo.enabledExtensionCount   = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    // Create logical device
    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
      kst::core::Logger::error("Failed to create logical device");
      return false;
    }

    // Get queue handles
    vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);

    kst::core::Logger::info("Logical device created successfully");
    return true;
  }

  auto VulkanContext::createSwapchain(uint32_t width, uint32_t height) -> bool {
    kst::core::Logger::info<uint32_t, uint32_t>(
        "Creating swapchain with dimensions: {} x {}",
        width,
        height
    );

    // Check for valid physical device and surface
    if (m_physicalDevice == VK_NULL_HANDLE || m_surface == VK_NULL_HANDLE) {
      kst::core::Logger::error("Cannot create swapchain: invalid physical device or surface");
      return false;
    }

    // Query surface capabilities
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &capabilities);

    // Query supported surface formats
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, nullptr);

    if (formatCount == 0) {
      kst::core::Logger::error("No surface formats supported");
      return false;
    }

    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, formats.data());

    // Query supported present modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        m_physicalDevice,
        m_surface,
        &presentModeCount,
        nullptr
    );

    if (presentModeCount == 0) {
      kst::core::Logger::error("No present modes supported");
      return false;
    }

    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        m_physicalDevice,
        m_surface,
        &presentModeCount,
        presentModes.data()
    );

    // Choose the best format (prefer SRGB if available)
    VkSurfaceFormatKHR surfaceFormat = formats[0];
    for (const auto& format : formats) {
      if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
          format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        surfaceFormat = format;
        break;
      }
    }

    m_swapchainFormat = surfaceFormat.format;

    // Choose present mode (prefer mailbox/triple buffering if available, fallback to FIFO)
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR; // Guaranteed to be available
    for (const auto& mode : presentModes) {
      if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
        presentMode = mode;
        break;
      }
    }

    // Choose swap extent (resolution of the swapchain images)
    VkExtent2D extent;
    if (capabilities.currentExtent.width != UINT32_MAX) {
      // If the surface has a defined size, use it
      extent = capabilities.currentExtent;
    } else {
      // Otherwise use the provided width and height, clamped to the allowed range
      extent.width =
          std::clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
      extent.height = std::clamp(
          height,
          capabilities.minImageExtent.height,
          capabilities.maxImageExtent.height
      );
    }

    m_swapchainExtent = extent;

    // Determine the number of images in the swapchain
    uint32_t imageCount =
        capabilities.minImageCount + 1; // Request at least one more than the minimum
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
      imageCount = capabilities.maxImageCount;
    }

    // Create the swapchain
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface          = m_surface;
    createInfo.minImageCount    = imageCount;
    createInfo.imageFormat      = surfaceFormat.format;
    createInfo.imageColorSpace  = surfaceFormat.colorSpace;
    createInfo.imageExtent      = extent;
    createInfo.imageArrayLayers = 1; // Always 1 unless developing a stereoscopic 3D app
    createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Handle queue families
    QueueFamilyIndices indices    = findQueueFamilies(m_physicalDevice, m_surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
      // If the graphics and present queues are from different families,
      // we need to share the images between them
      createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
      createInfo.queueFamilyIndexCount = 2;
      createInfo.pQueueFamilyIndices   = queueFamilyIndices;
    } else {
      // If they're the same family, we can use exclusive mode for better performance
      createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
      createInfo.queueFamilyIndexCount = 0;
      createInfo.pQueueFamilyIndices   = nullptr;
    }

    createInfo.preTransform   = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode    = presentMode;
    createInfo.clipped        = VK_TRUE;
    createInfo.oldSwapchain   = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapchain) != VK_SUCCESS) {
      kst::core::Logger::error("Failed to create swapchain");
      return false;
    }

    // Get the swapchain images
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
    m_swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_swapchainImages.data());

    kst::core::Logger::info<uint32_t>("Created swapchain with {} images", imageCount);
    return true;
  }

  auto VulkanContext::createImageViews() -> bool {
    kst::core::Logger::info("Creating image views");

    if (m_swapchainImages.empty()) {
      kst::core::Logger::error("No swapchain images available for image view creation");
      return false;
    }

    // Resize the container to hold the image views
    m_swapchainImageViews.resize(m_swapchainImages.size());

    // Create an image view for each swapchain image
    for (size_t i = 0; i < m_swapchainImages.size(); i++) {
      VkImageViewCreateInfo createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      createInfo.image = m_swapchainImages[i];

      // Specify how to interpret the image data
      createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
      createInfo.format   = m_swapchainFormat;

      // Default color component mapping
      createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

      // Image purpose and which part of the image to access
      createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
      createInfo.subresourceRange.baseMipLevel   = 0;
      createInfo.subresourceRange.levelCount     = 1;
      createInfo.subresourceRange.baseArrayLayer = 0;
      createInfo.subresourceRange.layerCount     = 1;

      // Create the image view
      if (vkCreateImageView(m_device, &createInfo, nullptr, &m_swapchainImageViews[i]) !=
          VK_SUCCESS) {
        kst::core::Logger::error<size_t>("Failed to create image view for swapchain image {}", i);
        return false;
      }
    }

    kst::core::Logger::info<size_t>(
        "Created {} swapchain image views",
        m_swapchainImageViews.size()
    );
    return true;
  }

  auto VulkanContext::createCommandPool() -> bool {
    kst::core::Logger::info("Creating command pool");

    // Get queue family indices
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_physicalDevice, m_surface);

    // Create a command pool for the graphics queue family
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Allow command buffers to be
                                                                      // reset individually
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
      kst::core::Logger::error("Failed to create command pool");
      return false;
    }

    kst::core::Logger::info("Command pool created successfully");
    return true;
  }

  auto VulkanContext::createCommandBuffers() -> bool {
    kst::core::Logger::info("Creating command buffers");

    // Create one command buffer for each swapchain image
    m_commandBuffers.resize(m_swapchainImages.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool        = m_commandPool;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

    if (vkAllocateCommandBuffers(m_device, &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
      kst::core::Logger::error("Failed to allocate command buffers");
      return false;
    }

    kst::core::Logger::info<size_t>("Created {} command buffers", m_commandBuffers.size());
    return true;
  }

  auto VulkanContext::createSyncObjects() -> bool {
    kst::core::Logger::info("Creating synchronization objects");

    // Define how many frames we want to process concurrently
    constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    // Resize the containers to hold our sync objects
    m_imageAvailalableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    // Create semaphores and fences for each frame
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Start in signaled state so first wait works

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailalableSemaphores[i]) !=
              VK_SUCCESS ||
          vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) !=
              VK_SUCCESS ||
          vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS) {
        kst::core::Logger::error("Failed to create synchronization objects for frame");
        return false;
      }
    }

    kst::core::Logger::info("Created synchronization objects successfully");
    return true;
  }

  void VulkanContext::cleanupSwapchain() {
    kst::core::Logger::info("Cleaning up swapchain");

    // Destroy image views - check nulls to avoid errors during shutdown
    for (auto& imageView : m_swapchainImageViews) {
      if (imageView != VK_NULL_HANDLE && m_device != VK_NULL_HANDLE) {
        vkDestroyImageView(m_device, imageView, nullptr);
        imageView = VK_NULL_HANDLE;
      }
    }
    m_swapchainImageViews.clear();

    // Destroy swapchain
    if (m_swapchain != VK_NULL_HANDLE && m_device != VK_NULL_HANDLE) {
      vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
      m_swapchain = VK_NULL_HANDLE;
    }

    m_swapchainImages.clear();
  }

  auto VulkanContext::beginFrame() -> uint32_t {
    // Wait for the previous frame to finish
    vkWaitForFences(m_device, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

    // Acquire the next swapchain image
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        m_device,
        m_swapchain,
        UINT64_MAX,
        m_imageAvailalableSemaphores[m_currentFrame],
        VK_NULL_HANDLE,
        &imageIndex
    );

    // Handle swapchain recreation if needed
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      // Swapchain is out of date, need to recreate
      recreateSwapchain();
      return beginFrame(); // Try again
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
      kst::core::Logger::error("Failed to acquire swapchain image");
      return 0;
    }

    // Reset the fence for the current frame
    vkResetFences(m_device, 1, &m_inFlightFences[m_currentFrame]);

    // Reset the command buffer for the current frame
    vkResetCommandBuffer(m_commandBuffers[imageIndex], 0);

    kst::core::Logger::debug<uint32_t>("Begin frame, acquired image index: {}", imageIndex);

    // Store the image index for use in endFrame
    m_currentImageIndex = imageIndex;

    return imageIndex;
  }

  void VulkanContext::endFrame() {
    // Submit the command buffer
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // Wait for the image to be available before drawing
    VkSemaphore waitSemaphores[]      = {m_imageAvailalableSemaphores[m_currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount     = 1;
    submitInfo.pWaitSemaphores        = waitSemaphores;
    submitInfo.pWaitDstStageMask      = waitStages;

    // Specify which command buffer to submit
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &m_commandBuffers[m_currentImageIndex];

    // Specify which semaphore to signal when we're done
    VkSemaphore signalSemaphores[]  = {m_renderFinishedSemaphores[m_currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = signalSemaphores;

    // Submit the command buffer to the graphics queue with the fence
    if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]) !=
        VK_SUCCESS) {
      kst::core::Logger::error("Failed to submit draw command buffer");
      return;
    }

    // Present the image
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    // Wait for rendering to be finished
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = signalSemaphores;

    // Specify which swapchain to present to
    VkSwapchainKHR swapchains[] = {m_swapchain};
    presentInfo.swapchainCount  = 1;
    presentInfo.pSwapchains     = swapchains;
    presentInfo.pImageIndices   = &m_currentImageIndex;

    // Present the image to the display
    VkResult result = vkQueuePresentKHR(m_presentQueue, &presentInfo);

    // Handle swapchain recreation if needed
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
      recreateSwapchain();
    } else if (result != VK_SUCCESS) {
      kst::core::Logger::error("Failed to present swapchain image");
    }

    // Move to the next frame
    m_currentFrame = (m_currentFrame + 1) % m_inFlightFences.size();

    kst::core::Logger::debug("End frame");
  }

  void VulkanContext::recreateSwapchain() {
    kst::core::Logger::info("Recreating swapchain");

    // Wait for the device to finish all operations
    vkDeviceWaitIdle(m_device);

    // Clean up old swapchain
    cleanupSwapchain();

    // Create new swapchain
    createSwapchain(m_swapchainExtent.width, m_swapchainExtent.height);
    createImageViews();

    kst::core::Logger::info("Swapchain recreated successfully");
  }

  void VulkanContext::resize(uint32_t width, uint32_t height) {
    kst::core::Logger::info<uint32_t, uint32_t>("Resizing to {} x {}", width, height);

    // Store new dimensions
    m_swapchainExtent.width  = width;
    m_swapchainExtent.height = height;

    // Recreate the swapchain with new dimensions
    recreateSwapchain();
  }

  auto VulkanContext::createCommandRecorder() -> CommandRecorder* {
    kst::core::Logger::info("Creating command recorder");

    // Allocate a command buffer
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool        = m_commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    if (vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
      kst::core::Logger::error("Failed to allocate command buffer for recorder");
      return nullptr;
    }

    // Create and return a new VulkanCommandRecorder
    return new VulkanCommandRecorder(*this, commandBuffer);
  }

  void VulkanContext::waitForIdle() {
    // Wait for the device to complete all operations
    vkDeviceWaitIdle(m_device);
  }

  auto VulkanContext::getCurrentBackBuffer() -> ::kst::core::TextureHandle {
    // TODO: Implement back buffer retrieval

    ::kst::core::TextureHandle handle{};
    return handle;
  }

  auto VulkanContext::getSwapchainFormat() const -> ::kst::core::Format {
    // Convert Vulkan format to engine format
    switch (m_swapchainFormat) {
    case VK_FORMAT_B8G8R8A8_UNORM:
      return ::kst::core::Format::BGRA8_UNORM;
    case VK_FORMAT_R8G8B8A8_UNORM:
      return ::kst::core::Format::RGBA8_UNORM;
    default:
      return ::kst::core::Format::UNKNOWN;
    }
  }

  void VulkanContext::getViewportDimensions(uint32_t& width, uint32_t& height) const {
    width  = m_swapchainExtent.width;
    height = m_swapchainExtent.height;
  }

  auto VulkanContext::createBuffer(
      uint64_t size,
      kst::core::BufferUsageFlags usage [[maybe_unused]],
      ::kst::core::MemoryDomain memory [[maybe_unused]]
  ) -> ::kst::core::BufferHandle {
    kst::core::Logger::info<uint64_t>("Creating buffer of size {}", size);

    // TODO: Implement buffer creation

    ::kst::core::BufferHandle handle{};
    handle.id = getNextResourceId();
    return handle;
  }

  void VulkanContext::destroyBuffer(const ::kst::core::BufferHandle& buffer [[maybe_unused]]) {
    kst::core::Logger::info<uint64_t>("Destroying buffer {}", buffer.id);

    // TODO: Implement buffer destruction
  }

  auto VulkanContext::mapBuffer(const ::kst::core::BufferHandle& buffer [[maybe_unused]]) -> void* {
    // TODO: Implement buffer mapping

    return nullptr;
  }

  void VulkanContext::unmapBuffer(const ::kst::core::BufferHandle& buffer [[maybe_unused]]) {
    // TODO: Implement buffer unmapping
  }

  auto VulkanContext::createTexture(
      uint32_t width,
      uint32_t height,
      uint32_t depth,
      ::kst::core::Format format [[maybe_unused]],
      ::kst::core::TextureUsageFlags usage [[maybe_unused]],
      ::kst::core::MemoryDomain memory [[maybe_unused]]
  ) -> ::kst::core::TextureHandle {
    kst::core::Logger::info<uint32_t, uint32_t, uint32_t>(
        "Creating texture of size {} x {} x {}",
        width,
        height,
        depth
    );

    // TODO: Implement texture creation

    ::kst::core::TextureHandle handle{};
    handle.id = getNextResourceId();
    return handle;
  }

  void VulkanContext::destroyTexture(const ::kst::core::TextureHandle& texture [[maybe_unused]]) {
    kst::core::Logger::info<uint64_t>("Destroying texture {}", texture.id);

    // TODO: Implement texture destruction
  }

  auto VulkanContext::createSampler(
      kst::core::FilterMode minFilter [[maybe_unused]],
      kst::core::FilterMode magFilter [[maybe_unused]],
      kst::core::AddressMode addressU [[maybe_unused]],
      kst::core::AddressMode addressV [[maybe_unused]],
      kst::core::AddressMode addressW [[maybe_unused]]
  ) -> ::kst::core::SamplerHandle {
    kst::core::Logger::info("Creating sampler");

    // TODO: Implement sampler creation

    ::kst::core::SamplerHandle handle{};
    handle.id = getNextResourceId();
    return handle;
  }

  void VulkanContext::destroySampler(const ::kst::core::SamplerHandle& sampler [[maybe_unused]]) {
    kst::core::Logger::info<uint64_t>("Destroying sampler {}", sampler.id);

    // TODO: Implement sampler destruction
  }

  auto VulkanContext::createShader(
      ::kst::core::ShaderStage stage [[maybe_unused]],
      const void* code [[maybe_unused]],
      size_t codeSize
  ) -> ::kst::core::ShaderHandle {
    kst::core::Logger::info<size_t>("Creating shader with code size {}", codeSize);

    // TODO: Implement shader creation

    ::kst::core::ShaderHandle handle{};
    handle.id = getNextResourceId();
    return handle;
  }

  void VulkanContext::destroyShader(const ::kst::core::ShaderHandle& shader [[maybe_unused]]) {
    kst::core::Logger::info<uint64_t>("Destroying shader {}", shader.id);

    // TODO: Implement shader destruction
  }

  void VulkanContext::setObjectName(
      kst::core::ObjectType type [[maybe_unused]],
      uint64_t objectId [[maybe_unused]],
      std::string_view name
  ) {
    kst::core::Logger::info<std::string_view>("Setting object name to '{}'", name);

    // TODO: Implement object naming
  }

  auto VulkanContext::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
      -> uint32_t {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
      if (((typeFilter & (1 << i)) != 0u) &&
          (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
        return i;
      }
    }

    kst::core::Logger::error<>("Failed to find suitable memory type");
    return 0;
  }

  auto VulkanContext::beginSingleTimeCommands() -> VkCommandBuffer {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool        = m_commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
  }

  void VulkanContext::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &commandBuffer;

    vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_graphicsQueue);

    vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
  }

  auto VulkanContext::getNextResourceId() -> uint64_t {
    return m_nextResourceId++;
  }

  void VulkanContext::executeCommands(const command::RenderCommand* commands, size_t count) {
    if (count == 0 || commands == nullptr) {
      return;
    }

    kst::core::Logger::debug<size_t>("Executing {} render commands", count);

    // Use the current command buffer for the current frame
    VkCommandBuffer commandBuffer = m_commandBuffers[m_currentImageIndex];

    // Begin recording commands
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
      kst::core::Logger::error<>("Failed to begin recording command buffer");
      return;
    }

    // Process each command
    for (size_t i = 0; i < count; ++i) {
      const auto& cmd = commands[i];

      switch (cmd.type) {
      case command::RenderCommandType::CLEAR: {
        // Handle clear command
        const auto& clearData = cmd.clearData;

        VkClearValue clearValue{};
        if ((static_cast<uint8_t>(clearData.flags) &
             static_cast<uint8_t>(command::ClearFlags::COLOR)) != 0) {
          clearValue.color = {
              {clearData.color.r, clearData.color.g, clearData.color.b, clearData.color.a}
          };

          // Log the clear color for debugging
          kst::core::Logger::debug<float, float, float, float>(
              "Clear color: {}, {}, {}, {}",
              clearData.color.r,
              clearData.color.g,
              clearData.color.b,
              clearData.color.a
          );
        }

        if ((static_cast<uint8_t>(clearData.flags) &
             static_cast<uint8_t>(command::ClearFlags::DEPTH)) != 0) {
          clearValue.depthStencil.depth = clearData.depth;
        }

        if ((static_cast<uint8_t>(clearData.flags) &
             static_cast<uint8_t>(command::ClearFlags::STENCIL)) != 0) {
          clearValue.depthStencil.stencil = clearData.stencil;
        }

        // Set up the viewport and scissor
        VkViewport viewport{};
        viewport.x        = 0.0f;
        viewport.y        = 0.0f;
        viewport.width    = static_cast<float>(m_swapchainExtent.width);
        viewport.height   = static_cast<float>(m_swapchainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = m_swapchainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        // Clear the image - for now, we'll use a simple vkCmdClearColorImage
        // to clear the swapchain image directly
        if (m_currentImageIndex < m_swapchainImages.size()) {
          // Transition the image layout for clearing
          VkImageMemoryBarrier barrier{};
          barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
          barrier.oldLayout                       = VK_IMAGE_LAYOUT_UNDEFINED;
          barrier.newLayout                       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
          barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
          barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
          barrier.image                           = m_swapchainImages[m_currentImageIndex];
          barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
          barrier.subresourceRange.baseMipLevel   = 0;
          barrier.subresourceRange.levelCount     = 1;
          barrier.subresourceRange.baseArrayLayer = 0;
          barrier.subresourceRange.layerCount     = 1;
          barrier.srcAccessMask                   = 0;
          barrier.dstAccessMask                   = VK_ACCESS_TRANSFER_WRITE_BIT;

          vkCmdPipelineBarrier(
              commandBuffer,
              VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
              VK_PIPELINE_STAGE_TRANSFER_BIT,
              0,
              0,
              nullptr,
              0,
              nullptr,
              1,
              &barrier
          );

          // Clear the image
          VkClearColorValue clearColorValue{};
          clearColorValue.float32[0] = clearData.color.r;
          clearColorValue.float32[1] = clearData.color.g;
          clearColorValue.float32[2] = clearData.color.b;
          clearColorValue.float32[3] = clearData.color.a;

          VkImageSubresourceRange range{};
          range.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
          range.baseMipLevel   = 0;
          range.levelCount     = 1;
          range.baseArrayLayer = 0;
          range.layerCount     = 1;

          vkCmdClearColorImage(
              commandBuffer,
              m_swapchainImages[m_currentImageIndex],
              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
              &clearColorValue,
              1,
              &range
          );

          // Transition the image layout for presenting
          barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
          barrier.newLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
          barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
          barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

          vkCmdPipelineBarrier(
              commandBuffer,
              VK_PIPELINE_STAGE_TRANSFER_BIT,
              VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
              0,
              0,
              nullptr,
              0,
              nullptr,
              1,
              &barrier
          );
        }

        break;
      }

      case command::RenderCommandType::DRAW: {
        // Handle draw command
        const auto& drawData = cmd.drawData;

        // Find the mesh and material resources
        // TODO: Implement actual mesh and material binding

        // Set up the transform (usually done via push constants or uniform buffers)
        // TODO: Set push constants with transformation matrix

        // Issue the draw command
        // vkCmdDraw(commandBuffer, drawData.vertexCount, drawData.instanceCount, 0, 0);
        kst::core::Logger::debug<uint32_t, uint32_t>(
            "Draw command: {} vertices, {} instances",
            drawData.vertexCount,
            drawData.instanceCount
        );
        break;
      }

      case command::RenderCommandType::DRAW_INDEXED: {
        // TODO: Implement indexed drawing
        break;
      }

      case command::RenderCommandType::DISPATCH: {
        // TODO: Implement compute dispatch
        break;
      }

      case command::RenderCommandType::COPY: {
        // TODO: Implement resource copying
        break;
      }

      case command::RenderCommandType::SET_VIEWPORT: {
        // TODO: Implement viewport setting
        break;
      }

      case command::RenderCommandType::SET_SCISSOR: {
        // TODO: Implement scissor setting
        break;
      }

      default:
        kst::core::Logger::warn<int>("Unknown command type: {}", static_cast<int>(cmd.type));
        break;
      }
    }

    // End the command buffer recording
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
      kst::core::Logger::error("Failed to end recording command buffer");
    }
  }

  void VulkanContext::transitionResource(
      ::kst::renderer::resource::ResourceID resource,
      ::kst::core::ResourceState oldState,
      ::kst::core::ResourceState newState
  ) {
    if (oldState == newState) {
      return; // No transition needed
    }

    kst::core::Logger::debug<uint32_t, int, int>(
        "Transitioning resource {} from state {} to state {}",
        resource.index,
        static_cast<int>(oldState),
        static_cast<int>(newState)
    );

    // Find the resource type
    bool isBuffer = m_buffers.contains(resource);
    bool isImage  = m_images.contains(resource);

    if (!isBuffer && !isImage) {
      kst::core::Logger::warn<uint32_t>(
          "Resource {} not found for state transition",
          resource.index
      );
      return;
    }

    // Begin a command buffer for the transition
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    if (isBuffer) {
      // Handle buffer barrier
      VkBuffer buffer = m_buffers[resource];

      VkBufferMemoryBarrier barrier{};
      barrier.sType  = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
      barrier.buffer = buffer;
      barrier.offset = 0;
      barrier.size   = VK_WHOLE_SIZE;

      // Set source and destination access masks based on state transition
      barrier.srcAccessMask = convertResourceStateToAccessFlags(oldState);
      barrier.dstAccessMask = convertResourceStateToAccessFlags(newState);

      // Set queue family indices if transferring between queues
      barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

      // Determine pipeline stages based on states
      VkPipelineStageFlags srcStageMask = convertResourceStateToPipelineStage(oldState);
      VkPipelineStageFlags dstStageMask = convertResourceStateToPipelineStage(newState);

      // Record the barrier
      vkCmdPipelineBarrier(
          commandBuffer,
          srcStageMask,
          dstStageMask,
          0,
          0,
          nullptr,
          1,
          &barrier,
          0,
          nullptr
      );
    } else if (isImage) {
      // Handle image barrier
      VkImage image = m_images[resource];

      VkImageMemoryBarrier barrier{};
      barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
      barrier.image = image;

      // Set old and new layouts based on states
      barrier.oldLayout = convertResourceStateToImageLayout(oldState);
      barrier.newLayout = convertResourceStateToImageLayout(newState);

      // Set source and destination access masks
      barrier.srcAccessMask = convertResourceStateToAccessFlags(oldState);
      barrier.dstAccessMask = convertResourceStateToAccessFlags(newState);

      // Set queue family indices if transferring between queues
      barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

      // Set affected image subresource range
      barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // TODO: Handle depth/stencil
      barrier.subresourceRange.baseMipLevel   = 0;
      barrier.subresourceRange.levelCount     = VK_REMAINING_MIP_LEVELS;
      barrier.subresourceRange.baseArrayLayer = 0;
      barrier.subresourceRange.layerCount     = VK_REMAINING_ARRAY_LAYERS;

      // Determine pipeline stages based on states
      VkPipelineStageFlags srcStageMask = convertResourceStateToPipelineStage(oldState);
      VkPipelineStageFlags dstStageMask = convertResourceStateToPipelineStage(newState);

      // Record the barrier
      vkCmdPipelineBarrier(
          commandBuffer,
          srcStageMask,
          dstStageMask,
          0,
          0,
          nullptr,
          0,
          nullptr,
          1,
          &barrier
      );
    }

    // Submit the command buffer
    endSingleTimeCommands(commandBuffer);
  }
} // namespace kst::renderer::core
