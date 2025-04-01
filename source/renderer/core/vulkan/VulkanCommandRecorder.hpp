#pragma once

#include <cstdint>

#include <renderer/core/CommandRecorder.hpp>
#include <vulkan/vulkan_core.h>

#include "renderer/core/GraphicsContext.hpp"
#include "renderer/core/GraphicsType.hpp"

namespace kst::renderer::core {

  class VulkanContext;

  class VulkanCommandRecorder final : public CommandRecorder {
  public:
    VulkanCommandRecorder(VulkanContext& context, VkCommandBuffer commandBuffer);
    ~VulkanCommandRecorder() override;

    void beginRecording() override;

    void endRecording() override;

    void reset() override;

    void submit(bool waitForCompletion) override;



    void
    bindVertexBuffer(::kst::core::BufferHandle buffer [[maybe_unused]], uint32_t binding, uint64_t offset [[maybe_unused]]) override;

    void bindIndexBuffer(
        ::kst::core::BufferHandle buffer [[maybe_unused]],
        uint64_t offset [[maybe_unused]],
        bool use32BitIndices
    ) override;

    void bindPipeline(const ::kst::core::PipelineHandle& pipeline [[maybe_unused]]) override;

    void setViewport(
        uint32_t posX,
        uint32_t posY,
        uint32_t width,
        uint32_t height,
        float minDepth,
        float maxDepth
    ) override;

    void setScissor(uint32_t posX, uint32_t posY, uint32_t width, uint32_t height) override;

    void draw(
        uint32_t vertexCount,
        uint32_t instanceCount,
        uint32_t firstVertex,
        uint32_t firstInstance
    ) override;

    void drawIndexed(
        uint32_t indexCount,
        uint32_t instanceCount,
        uint32_t firstIndex,
        int32_t vertexOffset,
        uint32_t firstInstance
    ) override;

    void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;

    void copyBuffer(
        ::kst::core::BufferHandle srcBuffer [[maybe_unused]],
        ::kst::core::BufferHandle dstBuffer [[maybe_unused]],
        uint64_t srcOffset [[maybe_unused]],
        uint64_t dstOffset [[maybe_unused]],
        uint64_t size
    ) override;

    void copyTexture(
        ::kst::core::TextureHandle srcTexture [[maybe_unused]],
        ::kst::core::TextureHandle dstTexture [[maybe_unused]],
        const ::kst::core::TextureRegion& srcRegion,
        ::kst::core::TextureRegion& dstRegion
    ) override;

    void bufferBarrier(
        ::kst::core::BufferHandle buffer,
        ::kst::core::ResourceState oldState,
        ::kst::core::ResourceState newState
    ) override;

    void textureBarrier(
        ::kst::core::TextureHandle texture,
        ::kst::core::ResourceState oldState,
        ::kst::core::ResourceState newState
    ) override;

    void clearTexture(
        ::kst::core::TextureHandle texture,
        const ::kst::core::ClearValue& clearValue,
        bool isDepthStencil
    ) override;

    void beginRenderPass(
        ::kst::core::RenderPassHandle renderPass,
        ::kst::core::FramebufferHandle framebuffer
    ) override;

    void endRenderPass() override;

    void pushConstants(const void* data, size_t size, uint32_t offset) override;

  private:
    VulkanContext& m_context;
    VkCommandBuffer m_commandBuffer;
    bool m_isRecording = false;
  };

} // namespace kst::renderer::core
