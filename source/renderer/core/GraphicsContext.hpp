#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

#include "CoreTypes.hpp"
#include "renderer/commands/RenderCommand.hpp"
#include "renderer/core/GraphicsHandles.hpp"
#include "renderer/core/GraphicsType.hpp"
#include "renderer/resources/ResourceID.hpp"

namespace kst::renderer::core {
  class GraphicsDevice;
  class CommandRecorder;

  class GraphicsContext {
  public:
    virtual ~GraphicsContext() = default;

    /*
     *  Initialize the graphics context with the given window handle
     *
     *  Manages the lifecycle of the graphics device, resource creation,
     *  and submission of command buffers.
     */
    virtual auto initialize(void* windowHandle, uint32_t width, uint32_t height) -> bool = 0;

    /*
     *  Shutdown the graphics context and release any resources.
     */
    virtual void shutdown() = 0;

    /*
     *  Get the graphics device capabilities and properties.
     *
     *  @return Reference to the graphics device
     */
    virtual auto getDevice() -> const GraphicsDevice& = 0;

    /*
     *  Begin a new frame for rendering.
     *  This prepares the backend for rendering commands and acquires the next available swapchain
     * image.
     *
     *  @return The index of the acquired swapchain image
     */
    virtual auto beginFrame() -> uint32_t = 0;

    /*
     *  End the current frame and present the rendered image to the screen.
     *
     *  This submits all pending commands and presents the swapchain image.
     */
    virtual void endFrame() = 0;

    /*
     *  Handle Window resize events.
     *  Recreates the swapchain with the new dimensions.
     *
     *  @param width New width of the window
     *  @param height New height of the window
     */
    virtual void resize(uint32_t width, uint32_t height) = 0;

    /*
     *  Create a command recorder for recording graphics commands.
     *
     *  @return New command Recorder instance
     */
    virtual auto createCommandRecorder() -> CommandRecorder* = 0;

    /*
     *  Wait for the GPU to finish all pending operations.
     */
    virtual void waitForIdle() = 0;

    /*
     *  Get the current back buffer as a Texture
     *
     *  @return Handle to the current back buffer texture
     */
    virtual auto getCurrentBackBuffer() -> ::kst::core::TextureHandle = 0;

    /*
     *  Get the swapchain image format
     *
     * @return Format of the swapchain images
     */
    virtual auto getSwapchainFormat() const -> ::kst::core::Format = 0;

    virtual void
    registerSwapchainResource(const ::kst::renderer::resource::ResourceID& resource) = 0;

    /*
     *  Get the current viewport dimensions
     *
     *  @param width Output parameter to store the viewport width
     *  @param height Output parameter to store the viewport height
     */
    virtual void getViewportDimensions(uint32_t& width, uint32_t& height) const = 0;

    // Resource creation and management

    /*
     *  Create a buffer resource
     *
     *  @param size Size of the buffer in bytes
     *  @param usage How the buffer will be used
     *  @param memory Memory domain for the buffer
     *  @return Handle to the created buffer
     */
    virtual auto
    createBuffer(uint64_t size, kst::core::BufferUsageFlags usage, ::kst::core::MemoryDomain memory)
        -> ::kst::core::BufferHandle = 0;

    virtual void executeCommands(const command::RenderCommand* commands, size_t count) = 0;

    virtual void transitionResource(
        ::kst::renderer::resource::ResourceID resource,
        ::kst::core::ResourceState oldState,
        ::kst::core::ResourceState newState
    ) = 0;

    /*
     *  Destroy a buffer resource
     *
     *  @param buffer Handle to the buffer to destroy
     */
    virtual void destroyBuffer(const ::kst::core::BufferHandle& buffer) = 0;

    /*
     *  Map a buffer for GPU access.
     *
     *  @param buffer Handle to the buffer to Map
     *  @return Pointer to the mapped buffer data, or nullptr if mapping failed
     */
    virtual auto mapBuffer(const ::kst::core::BufferHandle& buffer) -> void* = 0;

    /*
     *  Unmap a previously mapped buffer
     *
     *  @param buffer Handle to the buffer to unmap
     */
    virtual void unmapBuffer(const ::kst::core::BufferHandle& buffer) = 0;

    /*
     *  Create a texture resource
     *
     *  @param width Width of the texture
     *  @param height Height of the Texture
     *  @param depth Depth of the texture (for 3D textures)
                                                 @param format Format of the texture
     *  @param usage How the texture will be used
     *  @param memory Memory domain for the texture
     *  @return Handle to the created texture
     */
    virtual auto createTexture(
        uint32_t width,
        uint32_t height,
        uint32_t depth,
        ::kst::core::Format format,
        ::kst::core::TextureUsageFlags usage,
        ::kst::core::MemoryDomain memory
    ) -> ::kst::core::TextureHandle = 0;

    /*
     *  Destroy a texture resource
     *
     *  @param texture Handle to the texture to destroy
     */
    virtual void destroyTexture(const ::kst::core::TextureHandle& texture) = 0;

    /*
     *  Create a texture for texture sampling
     *
     *  @param minFilter Minification filter mode
     *  @param magFilter Magnification filter mode
     *  @param addressU Addressing mode for U coordinates
     *  @param addressV Addressing mode for V coordinates
     *  @param addressW Addressing mode for W coordinates
     *  @return Handle to the created sampler
     */
    virtual auto createSampler(
        kst::core::FilterMode minFilter,
        kst::core::FilterMode magFilter,
        kst::core::AddressMode addressU,
        kst::core::AddressMode addressV,
        kst::core::AddressMode addressW
    ) -> ::kst::core::SamplerHandle = 0;

    /*
     *  Destroy a sampler.
     *
     *  @param sampler Handle to the sampler to destroy
     */
    virtual void destroySampler(const ::kst::core::SamplerHandle& sampler) = 0;

    /*
     *  Create a shader resource
     *
     *  @param stage Shader stage (vertex, fragment, compute)
     *  @param code Pointer to the shader code
     *  @param codeSize Size of the shader code
     *  @return Handle to the created shader
     */
    virtual auto createShader(::kst::core::ShaderStage stage, const void* code, size_t codeSize)
        -> ::kst::core::ShaderHandle = 0;

    /*
     *  Destroy a shader resource
     *
     *  @param shader Handle to the shader to destroy
     */
    virtual void destroyShader(const ::kst::core::ShaderHandle& shader) = 0;

    /*
     *  Set a debug name for a resource.
     *
     *  @param type Type of the resource
     *  @param Raw Handle of the resource
     *  @param name Name to set for the resource
     */
    virtual void
    setObjectName(kst::core::ObjectType type, uint64_t objectId, std::string_view name) = 0;
  };

} // namespace kst::renderer::core
