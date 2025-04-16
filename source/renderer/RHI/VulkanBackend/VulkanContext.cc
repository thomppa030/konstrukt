#include "VulkanContext.hpp"

#include <cstdint>
#include <string>

#include "VulkanBackend/VulkanCore/Context.hpp"
#include "core/Logger.hpp"

namespace kst::renderer {
  VulkanContext::VulkanContext(const ContextOptions& options) {
    initVulkan(options);
  }

  VulkanContext::~VulkanContext() {}

  void VulkanContext::initVulkan(const ContextOptions& options) {
    std::vector<std::string> instanceExtensions;
    setupInstanceExtension(instanceExtensions, options);

    std::vector<std::string> deviceExtensions;
    setupDeviceExtension(deviceExtensions, options);

    std::vector<std::string> validationLayers;
    setupValidationLayers(validationLayers, options);

    if (options.enableRayTracing) {
      VulkanCore::Context::enableRayTracingFeatures();
    }

    VulkanCore::Context::enableDefaultFeatures();
    VulkanCore::Context::enableScalarLayoutFeatures();
    VulkanCore::Context::enableBufferDeviceAddressFeature();
    VulkanCore::Context::enableDynamicRenderingFeature();
    VulkanCore::Context::enableSynchronization2Feature();

    m_context = std::make_unique<VulkanCore::Context>(
        options.window,
        validationLayers,
        instanceExtensions,
        deviceExtensions,
        VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT,
        options.printEnumerations,
        options.enableRayTracing,
        "Konstrukt Engine"
    );

    KST_CORE_INFO(
        "VulkanContext initialized with {} instance Extensions, {} device Extensions and {} "
        "validation Layers",
        instanceExtensions.size(),
        deviceExtensions.size(),
        validationLayers.size()
    );
  }

  void VulkanContext::setupInstanceExtension(
      std::vector<std::string>& extensions,
      const ContextOptions& options
  ) {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    for (uint32_t i = 0; i < glfwExtensionCount; i++) {
      extensions.emplace_back(glfwExtensions[i]);
    }

    extensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);

    if (options.enableValidation) {
      extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    extensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
  }

  void VulkanContext::setupDeviceExtension(
      std::vector<std::string>& extensions,
      const ContextOptions& options
  ) {
    // Required extensions
    extensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    // Shader related extensions
    extensions.emplace_back(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
    extensions.emplace_back(VK_KHR_16BIT_STORAGE_EXTENSION_NAME);
    extensions.emplace_back(VK_KHR_8BIT_STORAGE_EXTENSION_NAME);

    // Advanced features
    extensions.emplace_back(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    extensions.emplace_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
    extensions.emplace_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    extensions.emplace_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);

    // Ray tracing
    if (options.enableRayTracing) {
      extensions.emplace_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
      extensions.emplace_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
      extensions.emplace_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
      extensions.emplace_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
    }
  }

  void VulkanContext::setupValidationLayers(
      std::vector<std::string>& layers,
      const ContextOptions& options
  ) {
    if (options.enableValidation) {
      layers.emplace_back("VK_LAYER_KHRONOS_VALIDATION");
    }
  }

  auto VulkanContext::createSurface(const SurfaceDescriptor& descriptor) -> bool {
    if (!descriptor.nativeWindowHandle) {
      KST_CORE_ERROR("Invalid window handle provided to createSurface");
      return false;
    }

    try {
      m_context->createSwapchain(
          VK_FORMAT_B8G8R8A8_SRGB,
          VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
          VK_PRESENT_MODE_MAILBOX_KHR,
          {descriptor.width, descriptor.height}
      );

      KST_CORE_INFO("Created swapchain with dimensions {}x{}", descriptor.width, descriptor.height);

      return true;
    } catch (const std::exception& e) {
      KST_CORE_ERROR("Failed to create swapchain: {}", e.what());
      return false;
    }
  }

  auto VulkanContext::createBuffer(
      uint64_t size,
      uint64_t usage,
      bool hostVisible,
      const std::string& name
  ) -> std::shared_ptr<Buffer> {}

  auto VulkanContext::createTexture(
      uint32_t width,
      uint32_t height,
      uint32_t format,
      uint32_t usage,
      const std::string& name
  ) -> std::shared_ptr<Buffer> {
    return nullptr;
  }

  auto VulkanContext::createSampler(const std::string& name) -> std::shared_ptr<Sampler> {
    return nullptr;
  }

  auto VulkanContext::createCommandQueue(uint32_t queueType, const std::string& name)
      -> std::shared_ptr<CommandQueue> {
    return nullptr;
  }

  auto VulkanContext::beginFrame() -> uint32_t {
    return 0;
  }

  void VulkanContext::endFrame() {}

  void VulkanContext::waitIdle() {
    if (m_context) {
      vkDeviceWaitIdle(m_context->device());
    }
  }

  auto VulkanContext::supportsFeature(uint32_t featureID) const -> bool {
    return UINT32_MAX;
  }

} // namespace kst::renderer
