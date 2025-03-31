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
    shutdown();
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
    if (m_device != VK_NULL_HANDLE) {
      vkDeviceWaitIdle(m_device);
    }

    for (size_t i = 0; i < m_imageAvailalableSemaphores.size(); i++) {
      vkDestroySemaphore(m_device, m_imageAvailalableSemaphores[i], nullptr);
      vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
      vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
    }

    if (m_commandPool != VK_NULL_HANDLE) {
      vkDestroyCommandPool(m_device, m_commandPool, nullptr);
    }

    cleanupSwapchain();

    if (m_device != VK_NULL_HANDLE) {
      vkDestroyDevice(m_device, nullptr);
      m_device = VK_NULL_HANDLE;
    }

    if (m_surface != VK_NULL_HANDLE) {
      vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
      m_surface = VK_NULL_HANDLE;
    }

    if (m_debugMessenger != VK_NULL_HANDLE) {
      auto func = (PFN_vkDestroyDebugUtilsMessengerEXT
      )vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");

      if (func != nullptr) {
        func(m_instance, m_debugMessenger, nullptr);
      }
      m_debugMessenger = VK_NULL_HANDLE;
    }

    if (m_instance != VK_NULL_HANDLE) {
      vkDestroyInstance(m_instance, nullptr);
    }
  }

  auto VulkanContext::getDevice() -> GraphicsDevice& {
    return m_vulkanDevice;
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
      kst::core::Logger::error<VkResult>("Failed to create Vulkan instance: {}", result);
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
        kst::core::Logger::error<VkResult>("Failed to set up debug messenger: {}", result);
        return false;
      }
    } else {
      kst::core::Logger::error<>("Could not find vkCreateDebugUtilsMessengerEXT function");
      return false;
    }

    return true;
  }

  auto VulkanContext::createSurface(void* windowHandle) -> bool {
    auto* window = reinterpret_cast<GLFWwindow*>(windowHandle);

#if defined(_WIN32) // Windows
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd      = glfwGetWin32Window(window);
    createInfo.hinstance = GetModuleHandle(nullptr);

    VkResult result = vkCreateWin32SurfaceKHR(m_instance, &createInfo, nullptr, &m_surface);
    if (result != VK_SUCCESS) {
      kst::core::Logger::error<>("Failed to create Win32 Vulkan surface: {}", result);
      return false;
    }
    kst::core::Logger::info<>("Created Vulkan surface using Win32");
#elif defined(__linux__) // Linux - detect Wayland or X11
    // Check if GLFW window is using Wayland
    if (glfwGetWindowAttrib(window, GLFW_PLATFORM) == GLFW_PLATFORM_WAYLAND) {
      VkWaylandSurfaceCreateInfoKHR createInfo{};
      createInfo.sType   = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
      createInfo.display = glfwGetWaylandDisplay();
      createInfo.surface = glfwGetWaylandWindow(window);

      VkResult result = vkCreateWaylandSurfaceKHR(m_instance, &createInfo, nullptr, &m_surface);
      if (result != VK_SUCCESS) {
        kst::core::Logger::error<VkResult>("Failed to create Wayland Vulkan surface: {}", result);
        return false;
      }

      kst::core::Logger::info<>("Created Vulkan surface using Wayland");
    } else {
      // Fallback to X11
      VkXlibSurfaceCreateInfoKHR createInfo{};
      createInfo.sType  = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
      createInfo.dpy    = glfwGetX11Display();
      createInfo.window = glfwGetX11Window(window);
      VkResult result   = vkCreateXlibSurfaceKHR(m_instance, &createInfo, nullptr, &m_surface);
      if (result != VK_SUCCESS) {
        kst::core::Logger::error<VkResult>("Failed to create X11 Vulkan surface: {}", result);
        return false;
      }

      kst::core::Logger::info<>("Created Vulkan surface using X11");
    }
#elif defined(__APPLE__) // macOS
    VkMacOSSurfaceCreateInfoMVK createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
    createInfo.pView = glfwGetCocoaWindow(window);

    VkResult result = vkCreateMacOSSurfaceMVK(m_instance, &createInfo, nullptr, &m_surface);
    if (result != VK_SUCCESS) {
      kst::core::Logger::error<VkResult>("Failed to create macOS Vulkan surface: {}", result);
      return false;
    }
    kst::core::Logger::info<>("Created Vulkan surface using macOS");
#else
    // Fallback to using GLFW's built-in surface creation
    VkResult result = glfwCreateWindowSurface(m_instance, window, nullptr, &m_surface);
    if (result != VK_SUCCESS) {
      kst::core::Logger::error<VkResult>("Failed to create Vulkan surface using GLFW: {}", result);
      return false;
    }
    kst::core::Logger::info<>("Created Vulkan surface using GLFW generic method");
#endif

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
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
      if (presentSupport == VK_TRUE) {
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
    // Get device properties and features
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    // Check if queue families supported
    QueueFamilyIndices indices = findQueueFamilies(device, surface);
    if (!indices.isComplete()) {
      return 0; // Not suitable
    }

    // Check if required extensions are supported
    bool extensionsSupported = checkDeviceExtensionSupport(device);
    if (!extensionsSupported) {
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
    // Get available physical devices
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
      kst::core::Logger::error<>("Failed to find GPUs with Vulkan support");
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
      kst::core::Logger::error<>("Failed to find a suitable GPU");
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
    QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice, m_surface);

    return true;
  }
} // namespace kst::renderer::core
