#pragma once

#include <renderer/core/GraphicsDevice.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "CoreTypes.hpp"
#include "renderer/core/GraphicsType.hpp"

namespace kst::renderer::core {
  class VulkanDevice : public GraphicsDevice {
  public:
    VulkanDevice();

    void initialize(VkPhysicalDevice physicalDevice);

    auto supportsFeature(kst::core::FeatureFlag feature) -> bool override;
    auto getMaxTextureSize() const -> uint32_t override;
    void getMaxComputeWorkGroups(uint32_t& maxX, uint32_t& maxY, uint32_t& maxZ) override;

    auto getDeviceLimits() const -> ::kst::core::Limits override;

    auto getMemoryProperties() const -> ::kst::core::MemoryProperties override;

    auto getDeviceName() const -> std::string override;
    auto getDeviceVendor() const -> std::string override;

    void getAPIVersion(uint32_t& major, uint32_t& minor, uint32_t& patch) override;
    auto getDeviceType() const -> ::kst::core::DeviceType override;
    auto getAvailableMemory() -> uint64_t override;

  private:
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties m_properties{};
    VkPhysicalDeviceFeatures m_features{};
    VkPhysicalDeviceMemoryProperties m_memoryProperties{};
  };
} // namespace kst::renderer::core
