#include "VulkanDevice.hpp"

#include <cstdint>

#include <vulkan/vulkan_core.h>

#include "CoreTypes.hpp"

namespace kst::renderer::core {

  void VulkanDevice::initialize(VkPhysicalDevice physicalDevice) {
    m_physicalDevice = physicalDevice;

    vkGetPhysicalDeviceProperties(m_physicalDevice, &m_properties);

    vkGetPhysicalDeviceFeatures(m_physicalDevice, &m_features);

    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_memoryProperties);
  }

  auto VulkanDevice::supportsFeature(kst::core::FeatureFlag feature) -> bool {
    switch (feature) {
    case ::kst::core::FeatureFlag::COMPUTE_SHADERS:
      return true; // All Vulkan devices support compute shaders

    case ::kst::core::FeatureFlag::TESSELLATION_SHADERS:
      return m_features.tessellationShader == VK_TRUE;

    case ::kst::core::FeatureFlag::GEOMETRY_SHADER:
      return m_features.geometryShader == VK_TRUE;

    case ::kst::core::FeatureFlag::SHADER_FLOAT64:
      return m_features.shaderFloat64 == VK_TRUE;

    case ::kst::core::FeatureFlag::SHADER_INT64:
      return m_features.shaderInt64 == VK_TRUE;

    case ::kst::core::FeatureFlag::SHADER_INT16:
      return m_features.shaderInt16 == VK_TRUE;

    case ::kst::core::FeatureFlag::TEXTURE_CUBE_ARRAY:
      return m_features.imageCubeArray == VK_TRUE;

    case ::kst::core::FeatureFlag::SAMPLER_ANISOTROPY:
      return m_features.samplerAnisotropy == VK_TRUE;

    case ::kst::core::FeatureFlag::TEXTURE_COMPRESSION_BC:
      return m_features.textureCompressionBC == VK_TRUE;

    case ::kst::core::FeatureFlag::TEXTURE_COMPRESSION_ASTC:
      return m_features.textureCompressionASTC_LDR == VK_TRUE;

    case ::kst::core::FeatureFlag::TEXTURE_COMPRESSION_ETC2:
      return m_features.textureCompressionETC2 == VK_TRUE;

    case ::kst::core::FeatureFlag::MULTI_VIEWPORT_SUPPORT:
      return m_features.multiViewport == VK_TRUE;

    case ::kst::core::FeatureFlag::DEPTH_CLAMPING:
      return m_features.depthClamp == VK_TRUE;

    case ::kst::core::FeatureFlag::DEPTH_BIAS_CLAMP:
      return m_features.depthBiasClamp == VK_TRUE;

    case ::kst::core::FeatureFlag::DEPTH_BOUNDS:
      return m_features.depthBounds == VK_TRUE;

    case ::kst::core::FeatureFlag::WIDE_LINES:
      return m_features.wideLines == VK_TRUE;

    case ::kst::core::FeatureFlag::FILL_MODE_NON_SOLID:
      return m_features.fillModeNonSolid == VK_TRUE;

    case ::kst::core::FeatureFlag::INDEPENDENT_BLEND:
      return m_features.independentBlend == VK_TRUE;

    case ::kst::core::FeatureFlag::DUAL_SRC_BLEND:
      return m_features.dualSrcBlend == VK_TRUE;

    case ::kst::core::FeatureFlag::LOGIC_OP:
      return m_features.logicOp == VK_TRUE;

    case ::kst::core::FeatureFlag::SAMPLE_RATE_SHADING:
      return m_features.sampleRateShading == VK_TRUE;

    case ::kst::core::FeatureFlag::FULL_DRAW_INDEX_UINT32:
      return m_features.fullDrawIndexUint32 == VK_TRUE;

    case ::kst::core::FeatureFlag::MULTI_DRAW_INDIRECT:
      return m_features.multiDrawIndirect == VK_TRUE;

    case ::kst::core::FeatureFlag::DRAW_INDIRECT_FIRST_INSTANCE:
      return m_features.drawIndirectFirstInstance == VK_TRUE;

    case ::kst::core::FeatureFlag::OCCLUSION_QUERY_PRECISE:
      return m_features.occlusionQueryPrecise == VK_TRUE;

    case ::kst::core::FeatureFlag::PIPELINE_STATISTICS_QUERY:
      return m_features.pipelineStatisticsQuery == VK_TRUE;

    case ::kst::core::FeatureFlag::SHADER_STORES_AND_ATOMICS:
      return m_features.vertexPipelineStoresAndAtomics == VK_TRUE &&
             m_features.fragmentStoresAndAtomics == VK_TRUE;

    case ::kst::core::FeatureFlag::SHADER_CLIP_DISTANCE:
      return m_features.shaderClipDistance == VK_TRUE;

    case ::kst::core::FeatureFlag::SHADER_CULL_DISTANCE:
      return m_features.shaderCullDistance == VK_TRUE;

    case ::kst::core::FeatureFlag::SHADER_RESOURCE_RESIDENCY:
      return m_features.shaderResourceResidency == VK_TRUE;

    case ::kst::core::FeatureFlag::SHADER_RESOURCE_MIN_LOD:
      return m_features.shaderResourceMinLod == VK_TRUE;

    // Handle Vulkan 1.2+ features and extensions separately
    case ::kst::core::FeatureFlag::MESH_SHADER:
      // Would need to check for extension or Vulkan 1.2+ capability
      return false;

    default:
      return false;
    }
  }

  auto VulkanDevice::getMaxTextureSize() const -> uint32_t {
    return m_properties.limits.maxImageDimension2D;
  }

  void VulkanDevice::getMaxComputeWorkGroups(uint32_t& maxX, uint32_t& maxY, uint32_t& maxZ) {
    maxX = m_properties.limits.maxComputeWorkGroupCount[0];
    maxY = m_properties.limits.maxComputeWorkGroupCount[1];
    maxZ = m_properties.limits.maxComputeWorkGroupCount[2];
  }

  auto VulkanDevice::getDeviceLimits() const -> kst::core::Limits {
    ::kst::core::Limits limits{};

    limits.maxImageDimension1D           = m_properties.limits.maxImageDimension1D;
    limits.maxImageDimension2D           = m_properties.limits.maxImageDimension2D;
    limits.maxImageDimension3D           = m_properties.limits.maxImageDimension3D;
    limits.maxImageDimensionCube         = m_properties.limits.maxImageDimensionCube;
    limits.maxImageArrayLayers           = m_properties.limits.maxImageArrayLayers;
    limits.maxTexelBufferElements        = m_properties.limits.maxTexelBufferElements;
    limits.maxUniformBufferRange         = m_properties.limits.maxUniformBufferRange;
    limits.maxStorageBufferRange         = m_properties.limits.maxStorageBufferRange;
    limits.maxPushConstantsSize          = m_properties.limits.maxPushConstantsSize;
    limits.maxMemoryAllocationCount      = m_properties.limits.maxMemoryAllocationCount;
    limits.maxSamplerAllocationCount     = m_properties.limits.maxSamplerAllocationCount;
    limits.maxBoundDescriptorSets        = m_properties.limits.maxBoundDescriptorSets;
    limits.maxPerStageDescriptorSamplers = m_properties.limits.maxPerStageDescriptorSamplers;
    limits.maxPerStageDescriptorUniformBuffers =
        m_properties.limits.maxPerStageDescriptorUniformBuffers;
    limits.maxPerStageDescriptorStorageBuffers =
        m_properties.limits.maxPerStageDescriptorStorageBuffers;
    limits.maxPerStageDescriptorSampledImages =
        m_properties.limits.maxPerStageDescriptorSampledImages;
    limits.maxPerStageDescriptorStorageImages =
        m_properties.limits.maxPerStageDescriptorStorageImages;
    limits.maxPerStageResources           = m_properties.limits.maxPerStageResources;
    limits.maxDescriptorSetSamplers       = m_properties.limits.maxDescriptorSetSamplers;
    limits.maxDescriptorSetUniformBuffers = m_properties.limits.maxDescriptorSetUniformBuffers;
    limits.maxDescriptorSetUniformBuffersDynamic =
        m_properties.limits.maxDescriptorSetUniformBuffersDynamic;
    limits.maxDescriptorSetStorageBuffers = m_properties.limits.maxDescriptorSetStorageBuffers;
    limits.maxDescriptorSetStorageBuffersDynamic =
        m_properties.limits.maxDescriptorSetStorageBuffersDynamic;
    limits.maxDescriptorSetSampledImages = m_properties.limits.maxDescriptorSetSampledImages;
    limits.maxDescriptorSetStorageImages = m_properties.limits.maxDescriptorSetSampledImages;
    limits.maxFramebufferWidth           = m_properties.limits.maxFramebufferWidth;
    limits.maxFramebufferHeight          = m_properties.limits.maxFramebufferHeight;
    limits.maxFramebufferLayers          = m_properties.limits.maxFramebufferLayers;
    limits.maxColorAttachments           = m_properties.limits.maxColorAttachments;
    limits.maxSamplerAnisotropy          = m_properties.limits.maxSamplerAnisotropy;

    return limits;
  }

  auto VulkanDevice::getMemoryProperties() const -> kst::core::MemoryProperties {
    kst::core::MemoryProperties memoryProperties{};
    memoryProperties.memoryHeapCount = m_memoryProperties.memoryHeapCount;
    memoryProperties.memoryTypeCount = m_memoryProperties.memoryTypeCount;

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
      auto& destType      = memoryProperties.memoryTypes[i];
      const auto& srcType = m_memoryProperties.memoryTypes[i];

      destType.heapIndex     = srcType.heapIndex;
      destType.isDeviceLocal = ((srcType.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0);
      destType.isHostVisible = ((srcType.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0);
      destType.isHostCoherent =
          ((srcType.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0);
      destType.isHostCached = ((srcType.propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) != 0);
    }

    return memoryProperties;
  }

  auto VulkanDevice::getDeviceName() const -> std::string {
    return m_properties.deviceName;
  }

  auto VulkanDevice::getDeviceVendor() const -> std::string {
    switch (m_properties.vendorID) {
    case 0x1002:
      return "AMD";
    case 0x1010:
      return "ImgTec";
    case 0x10DE:
      return "NVIDIA";
    case 0x13B5:
      return "ARM";
    case 0x5143:
      return "Qualcomm";
    case 0x8086:
      return "INTEL";
    default:
      return "UNKNOWN";
    }
  }

  void VulkanDevice::getAPIVersion(uint32_t& major, uint32_t& minor, uint32_t& patch) {
    major = VK_VERSION_MAJOR(m_properties.apiVersion);
    minor = VK_VERSION_MINOR(m_properties.apiVersion);
    patch = VK_VERSION_PATCH(m_properties.apiVersion);
  }

  auto VulkanDevice::getDeviceType() const -> kst::core::DeviceType {
    switch (m_properties.deviceType) {
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
      return kst::core::DeviceType::DISCRETE;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
      return kst::core::DeviceType::INTEGRATED;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
      return kst::core::DeviceType::SOFTWARE;
    default:
      return kst::core::DeviceType::DISCRETE;
    }
  }

  auto VulkanDevice::getAvailableMemory() -> uint64_t {
    uint64_t maxSize = 0;
    for (uint32_t i = 0; i < m_memoryProperties.memoryHeapCount; i++) {
      if ((m_memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0) {
        maxSize = std::max(maxSize, m_memoryProperties.memoryHeaps[i].size);
      }
    }
    return maxSize;
  }

} // namespace kst::renderer::core
