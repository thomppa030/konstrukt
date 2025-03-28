#pragma once

#include <cstdint>

#include "GraphicsHandles.hpp"
#include "GraphicsType.hpp"

namespace kst::renderer::core {
  class CommandRecorder {
  public:
    virtual ~CommandRecorder() = default;

    /*
     *  Begin recording commands.
     *  Must be called before any other recording methods
     */
    virtual void beginRecording() = 0;

    /*
     *  End recording commands.
     *  This must be called after all commands have been recorded
     */
    virtual void endRecording() = 0;

    /*
     *  Reset the command recorder to record new commands.
     *  This clears all previously recorded commands.
     */
    virtual void reset() = 0;

    /*
     *  Submit the recorded commands to the GPU for execution.
     *  This executes the commands and resets the recorder.
     *
     *  @param waitForCompletion Whether to block until execution completes
     */
    virtual void submit(bool waitForCompletion) = 0;

    /*
     *  Bind a vertex buffer for use in subsequent draw calls.
     *
     *  @param buffer Vertex buffer to Bind
     *  @param binding Binding slot to use
     *  @param offset Offset in bytes from the start of the buffer
     */
    virtual void
    bindVertexBuffer(::kst::core::BufferHandle buffer, uint32_t binding, uint64_t offset) = 0;

    /*
     *  Bind an index buffer for use in subsequent draw calls.
     *
     *  @param buffer Index buffer to Bind
     *  @param offset Offset in bytes from the start of the Buffer
     *  @param use32BitIndices Whether to use 32-bit indices (true) or 16-bit indices (false)
     */
    virtual void
    bindIndexBuffer(::kst::core::BufferHandle buffer, uint64_t offset, bool use32BitIndices) = 0;

    /*
     *  Bind a pipeline for use in subsequent draw calls.
     *
     *  @param pipeline Pipeline to Bind
     */
    virtual void bindPipeline(const ::kst::core::PipelineHandle& pipeline) = 0;

    /*
     *  Set the scissor rectangle for the subsequent draw calls.
     *
     *  @param posX X position of the viewport
     *  @param posY Y position of the viewport
     *  @param width Width of the viewport
     *  @param height Height of the viewport
     *  @param minDepth Minimum depth value for the viewport (typically 0.0)
     *  @param maxDepth Maximum depth value for the viewport (typically 1.0)
     */
    virtual void setViewport(
        uint32_t posX,
        uint32_t posY,
        uint32_t width,
        uint32_t height,
        float minDepth,
        float maxDepth
    ) = 0;

    /*
     *  Set the scissor rectangle for the subsequent draw calls.
     *
     *  @param posX X position of the scissor
     *  @param posY Y position of the scissor
     *  @param width Width of the scissor
     *  @param height Height of the scissor
     */
    virtual void setScissor(uint32_t posX, uint32_t posY, uint32_t width, uint32_t height) = 0;

    /*
     *  Draw primitives without an index buffer.
     *
     *  @param vertexCount Number of vertices to Draw
     *  @param instanceCount Number of instances to draw
     *  @param firstVertex First vertex to draw
     *  @param firstInstance First instance to draw
     */
    virtual void draw(
        uint32_t vertexCount,
        uint32_t instanceCount,
        uint32_t firstVertex,
        uint32_t firstInstance
    ) = 0;

    /*
     *  Draw primitives using an index buffer.
     *
     *  @param indexCount Number of indices to Draw
     *  @param instanceCount Number of instances to draw
     *  @param firstIndex First index to draw
     *  @param vertexOffset offset added to each index
     *  @param firstInstance First instance to draw
     */
    virtual void drawIndexed(
        uint32_t indexCount,
        uint32_t instanceCount,
        uint32_t firstIndex,
        int32_t vertexOffset,
        uint32_t firstInstance
    ) = 0;

    /*
     *  Dispatch a compute workgroup.
     *
     *  @param groupCountX Number of work groups in X dimension
     *  @param groupCountY Number of work groups in Y dimension
     *  @param groupCountZ Number of work groups in Z dimension
     */
    virtual void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;

    /*
     *  Copy Data from one buffer to another.
     *  @param srcBuffer Source buffer
     *  @param dstBuffer Destination buffer
     *  @param srcOffset Offset in bytes from start of source buffer
     *  @param dstOffset Offset in bytes from start of destination buffer
     *  @param size Size of the data to copy in bytes
     */
    virtual void copyBuffer(
        ::kst::core::BufferHandle srcBuffer,
        ::kst::core::BufferHandle dstBuffer,
        uint64_t srcOffset,
        uint64_t dstOffset,
        uint64_t size
    ) = 0;

    /*
     *  Copy data from one texture to another.
     *
     *  @param srcTexture Source Texture
     *  @param dstTexture Destination Texture
     *  @param srcRegion Region of source texture to copy from
     *  @param dstRegion Region of destination texture to copy to
     */
    virtual void copyTexture(
        ::kst::core::TextureHandle srcTexture,
        ::kst::core::TextureHandle dstTexture,
        const ::kst::core::TextureRegion& srcRegion,
        ::kst::core::TextureRegion& dstRegion
    ) = 0;

    /*
     *  Transition a buffer from one state to another.
     *  This ensures proper synchronization between operations.
     *
     *  @param buffer buffer to transtion
     *  @param oldState Current state of the buffer
     *  @param newState Desired state of the buffer
     */
    virtual void bufferBarrier(
        ::kst::core::BufferHandle buffer,
        ::kst::core::ResourceState oldState,
        ::kst::core::ResourceState newState
    ) = 0;

    /*
     *  Transition a texture from one state to another.
     *  This ensures proper synchronization between operations.
     *
     *  @param texture Texture to Transition
     *  @param oldState Current state of the texture
     *  @param newState Desired state of the texture
     */
    virtual void textureBarrier(
        ::kst::core::TextureHandle texture,
        ::kst::core::ResourceState oldState,
        ::kst::core::ResourceState newState
    ) = 0;

    /*
     *  Clear a texture with a specified clear value.
     *
     *  @param texture Texture to clear
     *  @param clearValue Clear value to use for the texture
     *  @param isDepthStencil Whether the texture is a depth or stencil texture (true)
     */
    virtual void clearTexture(
        ::kst::core::TextureHandle,
        const ::kst::core::ClearValue& clearValue,
        bool isDepthStencil
    ) = 0;

    /*
     *  Begin a render pass.
     *  This sets up rendering to one or more textures.
     *
     *  @param renderPass Render pass to begin
     *  @param framebuffer Framebuffer to use for rendering
     */
    virtual void beginRenderPass(
        ::kst::core::RenderPassHandle renderPass,
        ::kst::core::FramebufferHandle framebuffer
    ) = 0;

    /*
     *  End the current render pass.
     */
    virtual void endRenderPass() = 0;

    /*
     *  Push constants to the shader pipeline.
     *  Fast Way to send small amounts of data to the shader.
     *
     * @param data Pointer to constant data
     * @param size Size of the constant data in bytes
     * @param offset Offset in bytes into the push constant range
     */
    virtual void pushConstants(const void* data, size_t size, uint32_t offset) = 0;
  };
} // namespace kst::renderer::core
