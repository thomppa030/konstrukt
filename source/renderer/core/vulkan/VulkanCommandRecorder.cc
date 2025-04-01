#include "VulkanCommandRecorder.hpp"

#include <cstdint>
#include <stdexcept>

#include <vulkan/vulkan_core.h>

#include "VulkanContext.hpp"
#include "core/log/Logger.hpp"
#include "renderer/core/GraphicsHandles.hpp"
#include "renderer/core/GraphicsType.hpp"

namespace kst::renderer::core {

  VulkanCommandRecorder::VulkanCommandRecorder(
      VulkanContext& context,
      VkCommandBuffer commandBuffer
  )
      : m_context(context), m_commandBuffer(commandBuffer) {}

  VulkanCommandRecorder::~VulkanCommandRecorder() {
    // We should not be leaving command buffers in recording state
    if (m_isRecording) {
      endRecording();
    }
  }

  void VulkanCommandRecorder::beginRecording() {
    if (m_isRecording) {
      kst::core::Logger::warn("Attempting to begin recording when already recording");
      return; // we are already recording
    }

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(m_commandBuffer, &beginInfo) != VK_SUCCESS) {
      kst::core::Logger::error("Failed to begin recording command buffer");
      throw std::runtime_error("Failed to begin recording command buffer");
    }

    m_isRecording = true;
    kst::core::Logger::debug("Command buffer recording started");
  }

  void VulkanCommandRecorder::endRecording() {
    if (!m_isRecording) {
      kst::core::Logger::warn("Attempting to end recording when not recording");
      return;
    }

    if (vkEndCommandBuffer(m_commandBuffer) != VK_SUCCESS) {
      kst::core::Logger::error("Failed to end recording command buffer");
      throw std::runtime_error("Failed to end recording command buffer");
    }

    m_isRecording = false;
    kst::core::Logger::debug("Command buffer recording ended");
  }

  void VulkanCommandRecorder::reset() {
    if (m_isRecording) {
      endRecording();
    }

    if (vkResetCommandBuffer(m_commandBuffer, 0) != VK_SUCCESS) {
      kst::core::Logger::error("Failed to reset command buffer");
      throw std::runtime_error("Failed to reset command buffer");
    }

    kst::core::Logger::debug("Command buffer reset");
  }

  void VulkanCommandRecorder::submit(bool waitForCompletion) {
    if (m_isRecording) {
      endRecording();
    }

    VkSubmitInfo submitInfo       = {};
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &m_commandBuffer;

    // TODO: Get the actual graphics queue from the context
    auto* graphicsQueue = static_cast<VkQueue>(nullptr);

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
      kst::core::Logger::error("Failed to submit command buffer");
      throw std::runtime_error("Failed to submit command buffer");
    }

    if (waitForCompletion) {
      vkQueueWaitIdle(graphicsQueue);
      kst::core::Logger::debug("Command buffer submitted and execution completed");
    } else {
      kst::core::Logger::debug("Command buffer submitted for asynchronous execution");
    }
  }

  void VulkanCommandRecorder::bindVertexBuffer(
      ::kst::core::BufferHandle buffer [[maybe_unused]],
      uint32_t binding,
      uint64_t offset [[maybe_unused]]
  ) {
    if (!m_isRecording) {
      kst::core::Logger::error("Cannot bind vertex buffer: not in recording state");
      throw std::runtime_error("Cannot bind vertex buffer: not in recording state");
    }

    // TODO: Implement vertex buffer binding
    kst::core::Logger::info<uint32_t>("Binding vertex buffer at binding {}", binding);
  }

  void VulkanCommandRecorder::bindIndexBuffer(
      ::kst::core::BufferHandle buffer [[maybe_unused]],
      uint64_t offset [[maybe_unused]],
      bool use32BitIndices
  ) {
    if (!m_isRecording) {
      kst::core::Logger::error("Cannot bind index buffer: not in recording state");
      throw std::runtime_error("Cannot bind index buffer: not in recording state");
    }

    // TODO: Implement index buffer binding
    kst::core::Logger::info<const char*>(
        "Binding index buffer with {} indices",
        use32BitIndices ? "32-bit" : "16-bit"
    );
  }

  void VulkanCommandRecorder::bindPipeline(
      const ::kst::core::PipelineHandle& pipeline [[maybe_unused]]
  ) {
    if (!m_isRecording) {
      kst::core::Logger::error("Cannot bind pipeline: not in recording state");
      throw std::runtime_error("Cannot bind pipeline: not in recording state");
    }

    // TODO: Implement pipeline binding
    kst::core::Logger::info("Binding pipeline");
  }

  void VulkanCommandRecorder::setViewport(
      uint32_t posX,
      uint32_t posY,
      uint32_t width,
      uint32_t height,
      float minDepth,
      float maxDepth
  ) {
    if (!m_isRecording) {
      kst::core::Logger::error("Cannot set viewport: not in recording state");
      throw std::runtime_error("Cannot set viewport: not in recording state");
    }

    VkViewport viewport = {};
    viewport.x          = static_cast<float>(posX);
    viewport.y          = static_cast<float>(posY);
    viewport.width      = static_cast<float>(width);
    viewport.height     = static_cast<float>(height);
    viewport.minDepth   = minDepth;
    viewport.maxDepth   = maxDepth;

    vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);
    kst::core::Logger::debug<uint32_t, uint32_t, uint32_t, uint32_t, float, float>(
        "Set viewport to [{}x{} at {},{}, depth {}->{}]",
        width,
        height,
        posX,
        posY,
        minDepth,
        maxDepth
    );
  }

  void
  VulkanCommandRecorder::setScissor(uint32_t posX, uint32_t posY, uint32_t width, uint32_t height) {
    if (!m_isRecording) {
      kst::core::Logger::error("Cannot set scissor: not in recording state");
      throw std::runtime_error("Cannot set scissor: not in recording state");
    }

    VkRect2D scissor      = {};
    scissor.offset.x      = posX;
    scissor.offset.y      = posY;
    scissor.extent.width  = width;
    scissor.extent.height = height;

    vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
    kst::core::Logger::debug<uint32_t, uint32_t, uint32_t, uint32_t>(
        "Set scissor to [{}x{} at {},{}",
        width,
        height,
        posX,
        posY
    );
  }

  void VulkanCommandRecorder::draw(
      uint32_t vertexCount,
      uint32_t instanceCount,
      uint32_t firstVertex,
      uint32_t firstInstance
  ) {
    if (!m_isRecording) {
      kst::core::Logger::error("Cannot draw: not in recording state");
      throw std::runtime_error("Cannot draw: not in recording state");
    }

    vkCmdDraw(m_commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    kst::core::Logger::debug<uint32_t, uint32_t>(
        "Draw command: {} vertices, {} instances",
        vertexCount,
        instanceCount
    );
  }

  void VulkanCommandRecorder::drawIndexed(
      uint32_t indexCount,
      uint32_t instanceCount,
      uint32_t firstIndex,
      int32_t vertexOffset,
      uint32_t firstInstance
  ) {
    if (!m_isRecording) {
      kst::core::Logger::error("Cannot draw indexed: not in recording state");
      throw std::runtime_error("Cannot draw indexed: not in recording state");
    }

    vkCmdDrawIndexed(
        m_commandBuffer,
        indexCount,
        instanceCount,
        firstIndex,
        vertexOffset,
        firstInstance
    );
    kst::core::Logger::debug<uint32_t, uint32_t>(
        "Draw indexed command: {} indices, {} instances",
        indexCount,
        instanceCount
    );
  }

  void VulkanCommandRecorder::dispatch(
      uint32_t groupCountX,
      uint32_t groupCountY,
      uint32_t groupCountZ
  ) {
    if (!m_isRecording) {
      kst::core::Logger::error("Cannot dispatch: not in recording state");
      throw std::runtime_error("Cannot dispatch: not in recording state");
    }

    vkCmdDispatch(m_commandBuffer, groupCountX, groupCountY, groupCountZ);
    kst::core::Logger::debug<uint32_t, uint32_t, uint32_t>(
        "Dispatch command: {}x{}x{} workgroups",
        groupCountX,
        groupCountY,
        groupCountZ
    );
  }

  void VulkanCommandRecorder::copyBuffer(
      ::kst::core::BufferHandle srcBuffer [[maybe_unused]],
      ::kst::core::BufferHandle dstBuffer [[maybe_unused]],
      uint64_t srcOffset [[maybe_unused]],
      uint64_t dstOffset [[maybe_unused]],
      uint64_t size
  ) {
    if (!m_isRecording) {
      kst::core::Logger::error("Cannot copy buffer: not in recording state");
      throw std::runtime_error("Cannot copy buffer: not in recording state");
    }

    // TODO: Implement buffer copy
    kst::core::Logger::info<uint64_t>("Copying buffer, {} bytes", size);
  }

  void VulkanCommandRecorder::copyTexture(
      ::kst::core::TextureHandle srcTexture [[maybe_unused]],
      ::kst::core::TextureHandle dstTexture [[maybe_unused]],
      const ::kst::core::TextureRegion& srcRegion [[maybe_unused]],
      ::kst::core::TextureRegion& dstRegion [[maybe_unused]]
  ) {
    if (!m_isRecording) {
      kst::core::Logger::error("Cannot copy texture: not in recording state");
      throw std::runtime_error("Cannot copy texture: not in recording state");
    }

    // TODO: Implement texture copy
    kst::core::Logger::info("Copying texture region");
  }

  void VulkanCommandRecorder::bufferBarrier(
      ::kst::core::BufferHandle buffer [[maybe_unused]],
      ::kst::core::ResourceState oldState [[maybe_unused]],
      ::kst::core::ResourceState newState [[maybe_unused]]
  ) {
    if (!m_isRecording) {
      kst::core::Logger::error("Cannot insert buffer barrier: not in recording state");
      throw std::runtime_error("Cannot insert buffer barrier: not in recording state");
    }
  }

  void VulkanCommandRecorder::textureBarrier(
      ::kst::core::TextureHandle texture [[maybe_unused]],
      ::kst::core::ResourceState oldState [[maybe_unused]],
      ::kst::core::ResourceState newState [[maybe_unused]]
  ) {
    if (!m_isRecording) {
      kst::core::Logger::error("Cannot insert texture barrier: not in recording state");
      throw std::runtime_error("Cannot insert texture barrier: not in recording state");
    }
  }

  void VulkanCommandRecorder::clearTexture(
      ::kst::core::TextureHandle texture [[maybe_unused]],
      const ::kst::core::ClearValue& clearValue,
      bool isDepthStencil
  ) {
    if (!m_isRecording) {
      kst::core::Logger::error("Cannot clear texture: not in recording state");
      throw std::runtime_error("Cannot clear texture: not in recording state");
    }

    // TODO: Implement texture clearing
    if (isDepthStencil) {
      kst::core::Logger::info<float, uint8_t>(
          "Clearing depth-stencil texture: depth={}, stencil={}",
          clearValue.depthStencil.depth,
          clearValue.depthStencil.stencil
      );
    } else {
      kst::core::Logger::info("Clearing color texture");
    }
  }

  void VulkanCommandRecorder::beginRenderPass(
      ::kst::core::RenderPassHandle renderPass [[maybe_unused]],
      ::kst::core::FramebufferHandle framebuffer [[maybe_unused]]
  ) {
    if (!m_isRecording) {
      kst::core::Logger::error("Cannot begin render pass: not in recording state");
      throw std::runtime_error("Cannot begin render pass: not in recording state");
    }

    // TODO: Implement render pass beginning
    kst::core::Logger::info("Beginning render pass");
  }

  void VulkanCommandRecorder::endRenderPass() {
    if (!m_isRecording) {
      kst::core::Logger::error("Cannot end render pass: not in recording state");
      throw std::runtime_error("Cannot end render pass: not in recording state");
    }

    // TODO: Implement render pass ending
    kst::core::Logger::info("Ending render pass");
  }

  void VulkanCommandRecorder::pushConstants(
      const void* data [[maybe_unused]],
      size_t size,
      uint32_t offset
  ) {
    if (!m_isRecording) {
      kst::core::Logger::error("Cannot push constants: not in recording state");
      throw std::runtime_error("Cannot push constants: not in recording state");
    }

    // TODO: Implement push constants
    kst::core::Logger::info<size_t, uint32_t>(
        "Pushing {} bytes of constants at offset {}",
        size,
        offset
    );
  }

} // namespace kst::renderer::core
