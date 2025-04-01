#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <renderer/core/GraphicsContext.hpp>
#include <sys/types.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "VulkanCommandRecorder.hpp"
#include "VulkanDevice.hpp"
#include "renderer/core/GraphicsType.hpp"
#include "renderer/resources/ResourceID.hpp"

namespace kst::renderer::core {

  class VulkanContext final : public GraphicsContext {
  public:
    VulkanContext();
    ~VulkanContext() override;

    VulkanContext(const VulkanContext&)                    = delete;
    VulkanContext(VulkanContext&&)                         = delete;
    auto operator=(const VulkanContext&) -> VulkanContext& = delete;
    auto operator=(VulkanContext&&) -> VulkanContext&      = delete;

    auto initialize(void* windowHandle, uint32_t width, uint32_t height) -> bool override;
    void shutdown() override;

    auto getDevice() -> GraphicsDevice& override;

    auto beginFrame() -> uint32_t override;
    void endFrame() override;
    void resize(uint32_t width, uint32_t height) override;

    auto createCommandRecorder() -> CommandRecorder* override;

    auto getCurrentBackBuffer() -> ::kst::core::TextureHandle override;
    auto getSwapchainFormat() const -> ::kst::core::Format override;

    void registerSwapchainResource(const ::kst::renderer::resource::ResourceID& resource) override;

    void getViewportDimensions(uint32_t& width, uint32_t& height) const override;

    auto
    createBuffer(uint64_t size, kst::core::BufferUsageFlags usage, ::kst::core::MemoryDomain memory)
        -> ::kst::core::BufferHandle override;

    void executeCommands(const command::RenderCommand* commands, size_t count) override;

    void transitionResource(
        ::kst::renderer::resource::ResourceID resource,
        ::kst::core::ResourceState oldState,
        ::kst::core::ResourceState newState
    ) override;

    void destroyBuffer(const ::kst::core::BufferHandle& buffer) override;

    auto mapBuffer(const ::kst::core::BufferHandle& buffer) -> void* override;

    void unmapBuffer(const ::kst::core::BufferHandle& buffer) override;

    auto createTexture(
        uint32_t width,
        uint32_t height,
        uint32_t depth,
        ::kst::core::Format format,
        ::kst::core::TextureUsageFlags usage,
        ::kst::core::MemoryDomain memory
    ) -> ::kst::core::TextureHandle override;

    void destroyTexture(const ::kst::core::TextureHandle& texture) override;

    auto createSampler(
        kst::core::FilterMode minFilter,
        kst::core::FilterMode magFilter,
        kst::core::AddressMode addressU,
        kst::core::AddressMode addressV,
        kst::core::AddressMode addressW
    ) -> ::kst::core::SamplerHandle override;

    void destroySampler(const ::kst::core::SamplerHandle& sampler) override;

    auto createShader(::kst::core::ShaderStage stage, const void* code, size_t codeSize)
        -> ::kst::core::ShaderHandle override;

    void destroyShader(const ::kst::core::ShaderHandle& shader) override;

    void
    setObjectName(kst::core::ObjectType type, uint64_t objectId, std::string_view name) override;

    void waitForIdle() override;

  private:
    // Helper functions for resource state transitions
    auto convertResourceStateToAccessFlags(::kst::core::ResourceState state) -> VkAccessFlags;
    auto convertResourceStateToPipelineStage(::kst::core::ResourceState state)
        -> VkPipelineStageFlags;
    auto convertResourceStateToImageLayout(::kst::core::ResourceState state) -> VkImageLayout;

    VkInstance m_instance             = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device                 = VK_NULL_HANDLE;

    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;

    VkSurfaceKHR m_surface     = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainImageViews;
    VkFormat m_swapchainFormat   = VK_FORMAT_UNDEFINED;
    VkExtent2D m_swapchainExtent = {0, 0};

    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue  = VK_NULL_HANDLE;

    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_commandBuffers;

    std::vector<VkSemaphore> m_imageAvailalableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
    uint32_t m_currentFrame      = 0;
    uint32_t m_currentImageIndex = 0;

    std::unordered_map<resource::ResourceID, VkBuffer> m_buffers;
    std::unordered_map<resource::ResourceID, VkDeviceMemory> m_bufferMemories;
    std::unordered_map<resource::ResourceID, VkImage> m_images;
    std::unordered_map<resource::ResourceID, VkDeviceMemory> m_imageMemories;
    std::unordered_map<resource::ResourceID, VkImageView> m_imageViews;
    std::unordered_map<resource::ResourceID, VkSampler> m_samplers;
    std::unordered_map<resource::ResourceID, VkShaderModule> m_shaderModules;

    VulkanDevice m_vulkanDevice;

    auto createInstance() -> bool;
    auto setupDebugMessenger() -> bool;
    auto createSurface(void* windowHandle) -> bool;
    auto pickPhysicalDevice() -> bool;
    auto createLogicalDevice() -> bool;
    auto createSwapchain(uint32_t width, uint32_t height) -> bool;
    auto createImageViews() -> bool;
    auto createCommandPool() -> bool;
    auto createCommandBuffers() -> bool;
    auto createSyncObjects() -> bool;

    auto findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) -> uint32_t;

    auto beginSingleTimeCommands() -> VkCommandBuffer;
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    void cleanupSwapchain();
    void recreateSwapchain();

    auto getNextResourceId() -> uint64_t;
    uint64_t m_nextResourceId = 1;

    resource::ResourceID m_swapchainResource;
  };

} // namespace kst::renderer::core
