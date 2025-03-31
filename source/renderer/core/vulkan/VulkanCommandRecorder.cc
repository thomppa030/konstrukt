#include "VulkanCommandRecorder.hpp"

#include <iostream>
#include <stdexcept>

#include <vulkan/vulkan_core.h>

#include "log/Logger.hpp"
#include "renderer/core/GraphicsHandles.hpp"
#include "renderer/core/GraphicsType.hpp"

namespace kst::renderer::core {

  VulkanCommandRecorder::VulkanCommandRecorder(
      VulkanContext& context,
      VkCommandBuffer commandBuffer
  )
      : m_context(context), m_commandBuffer(commandBuffer) {}

  VulkanCommandRecorder::~VulkanCommandRecorder() {
    // We should not be leaving command Buffers in recording state
    if (m_isRecording) {
      endRecording();
    }
  }

  void VulkanCommandRecorder::beginRecording() {
    if (m_isRecording) {
      return; // we are already recording
    }

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(m_commandBuffer, &beginInfo) != VK_SUCCESS) {
      throw std::runtime_error("Failed to begin recording command buffer");
    }

    m_isRecording = true;
  }

  void VulkanCommandRecorder::reset() {
    if (m_isRecording) {
      endRecording();
    }

    vkResetCommandBuffer(m_commandBuffer, 0);
  }

  void VulkanCommandRecorder::submit(bool waitForCompletion) {
    if (m_isRecording) {
      endRecording();
    }

    VkSubmitInfo submitInfo       = {};
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &m_commandBuffer;

    auto* graphicsQueue = static_cast<VkQueue>(nullptr);

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
      throw std::runtime_error("Failed to subimt command buffer");
    }

    if (waitForCompletion) {
      vkQueueWaitIdle(graphicsQueue);
    }
  }

  void VulkanCommandRecorder::clearTexture(
      ::kst::core::TextureHandle texture,
      const ::kst::core::ClearValue& clearValue,
      bool isDepthStencil
  ) {
    if (m_isRecording) {
      endRecording();
    }

    // TODO is MVP Implementation
    // For the This stage of APP we just hardcode the clearvalue instead of looking up the image
    // from the texture handle

    if (!texture) {
      std::cout << "No Texture handling in the MVP at the moment" << '\n';
    }

    VkClearColorValue clearColor;
    clearColor.float32[0] = clearValue.color.r;
    clearColor.float32[1] = clearValue.color.g;
    clearColor.float32[2] = clearValue.color.b;
    clearColor.float32[3] = clearValue.color.a;

    VkImageSubresourceRange range = {};
    range.aspectMask              = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel            = 0;
    range.levelCount              = 1;
    range.baseArrayLayer          = 0;
    range.layerCount              = 1;

    if (isDepthStencil) {
      return;
    }
  }

  void VulkanCommandRecorder::bindVertexBuffer(
      ::kst::core::BufferHandle buffer,
      uint32_t binding,
      uint64_t offset
  ) {
    throw ::std::runtime_error("Function not implented yet!");
  }
  void VulkanCommandRecorder::bindIndexBuffer(
      ::kst::core::BufferHandle buffer,
      uint64_t offset,
      bool use32BitIndices
  ) {
    throw ::std::runtime_error("Function not implented yet!");
  }

  void bindPipeline(const ::kst::core::PipelineHandle& pipeline) {}

  void setViewport(
      uint32_t posX,
      uint32_t posY,
      uint32_t width,
      uint32_t height,
      float minDepth,
      float maxDepth
  ) {}

  void setScissor(uint32_t posX, uint32_t posY, uint32_t width, uint32_t height) {}
  void VulkanCommandRecorder::draw(
      uint32_t vertexCount,
      uint32_t instanceCount,
      uint32_t firstVertex,
      uint32_t firstInstance
  ) {}
  void VulkanCommandRecorder::drawIndexed(
      uint32_t indexCount,
      uint32_t instanceCount,
      uint32_t firstIndex,
      int32_t vertexOffset,
      uint32_t firstInstance
  ) {}

  void VulkanCommandRecorder::dispatch(
      uint32_t groupCountX,
      uint32_t groupCountY,
      uint32_t groupCountZ
  ) {}
  void VulkanCommandRecorder::copyBuffer(
      BufferHandle srcBuffer,
      BufferHandle dstBuffer,
      uint64_t srcOffset,
      uint64_t dstOffset,
      uint64_t size
  ) {}
  void VulkanCommandRecorder::copyTexture(
      TextureHandle srcTexture,
      TextureHandle dstTexture,
      const TextureRegion& srcRegion,
      const TextureRegion& dstRegion
  ) {}
  void VulkanCommandRecorder::bufferBarrier(
      BufferHandle buffer,
      ResourceState oldState,
      ResourceState newState
  ) {}
  void VulkanCommandRecorder::textureBarrier(
      TextureHandle texture,
      ResourceState oldState,
      ResourceState newState
  ) {}
  void VulkanCommandRecorder::beginRenderPass(
      RenderPassHandle renderPass,
      FramebufferHandle framebuffer
  ) {}
  void VulkanCommandRecorder::endRenderPass() {}
  void VulkanCommandRecorder::pushConstants(const void* data, uint32_t size, uint32_t offset) {}

} // namespace kst::renderer::core
