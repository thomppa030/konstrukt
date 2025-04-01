#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

#include "ResourceID.hpp"
#include "renderer/core/GraphicsType.hpp"
#include "renderer/framegraph/RenderPass.hpp"

namespace kst::renderer::resources {

  struct BufferDesc {
    size_t size       = 0;     // Size of the buffer in bytes
    bool hostVisible  = false; // Whether CPU can directly map and access this buffer
    bool hostCoherent = false; // Whether memory writes are automatically visible without flush

                               // Buffer usage flags (combine with bitwise OR)
    ::kst::core::BufferUsageFlags
        usage; // How this buffer will be used (vertex, index, uniform, storage, etc.)
               // Example: BufferUsage::Vertex | BufferUsage::TransferDst
  };

  struct TextureDesc {
    uint32_t width;       // Width in pixels
    uint32_t height;      // Height in pixels
    uint32_t depth = 1;   // Depth in pixels (for 3D textures, 1 for 2D textures)

    uint32_t mipLevels;   // Number of mip levels ( 1 = no mipmaps)

    uint32_t arrayLevels; // Number of array layers (1 = not an array texture)

    ::kst::core::Format format = ::kst::core::Format::UNKNOWN; // Pixel Format (RGBA8, R32F, etc.)

    ::kst::core::ResourceState
        usage; // How this texture will be used (sampled, storage, render target, etc.)
               // Example: TextureUsage::Sampled | TextureUsage::RenderTarget

    bool cubeMap = false;
  };

  struct RenderTargetDesc {
    uint32_t width  = 0;
    uint32_t height = 0;

    ::kst::core::Format format = ::kst::core::Format::UNKNOWN;

    bool clearOnLoad = true; // Whether to automatically clear this target when a render pass
                             // begins. Helps avoiding undefined values from previous rendering

    float clearColor[4] = {0.0F, 0.0F, 0.0F, 1.0F};
  };

  constexpr uint32_t DEFAULT_MAXTEXTURES = 1024;
  constexpr uint32_t DEFAULT_MAXBUFFERS  = 1024;
  constexpr uint32_t DEFAULT_MAXSAMPLERS = 1024;

  // clang-format off
  struct BindlessTableDesc {
    uint32_t maxTextures = DEFAULT_MAXTEXTURES; // Maximum number of textures accessible through this table.
                                                // Higher Values consume more GPU memory but allow more resources

    uint32_t maxBuffers =  DEFAULT_MAXBUFFERS;  // Maximum number of buffers accessible through this table

    uint32_t maxSamplers = DEFAULT_MAXSAMPLERS; // Maximum number of samplers accessible through this table.
                                                // Samplers control how textures are read (filtering, addressing)

    bool dynamicIndexing = true; // Whether shaders can use non-constant indices
                                 // Required only for truly dynamic resource access
  };
  // clang-format on

  struct ResourceDesc {
    ::kst::core::ResourceType type          = ::kst::core::ResourceType::BUFFER;
    ::kst::core::ResourceState initialState = ::kst::core::ResourceState::UNDEFINED;
    bool transient = false; // Wheter this is a temporary resource that exists only within one frame
                            // Transient resources can and should be optimized with memory aliasing

    union {
      BufferDesc bufferDesc{};
      TextureDesc textureDesc;
      RenderTargetDesc renderTargetDesc;
      BindlessTableDesc bindlessTableDesc;
    };

    ResourceDesc(const ResourceDesc&)                    = default;
    ResourceDesc(ResourceDesc&&)                         = delete;
    auto operator=(const ResourceDesc&) -> ResourceDesc& = default;
    auto operator=(ResourceDesc&&) -> ResourceDesc&      = delete;

    // TODO we try with union first, will test performance implications
    ResourceDesc() { new (&bufferDesc) BufferDesc{}; }

    ResourceDesc(
        ::kst::core::ResourceType type,
        const BufferDesc& desc,
        ::kst::core::ResourceState state = ::kst::core::ResourceState::UNDEFINED,
        bool isTransient                 = false
    )
        : type(type), initialState(state), transient(isTransient) {
      new (&bufferDesc) BufferDesc(desc);
    }

    ResourceDesc(
        ::kst::core::ResourceType type,
        const TextureDesc& desc,
        ::kst::core::ResourceState state = ::kst::core::ResourceState::UNDEFINED,
        bool isTransient                 = false
    )
        : type(type), initialState(state), transient(isTransient) {
      new (&textureDesc) TextureDesc(desc);
    }

    ResourceDesc(
        ::kst::core::ResourceType type,
        const RenderTargetDesc& desc,
        ::kst::core::ResourceState state = ::kst::core::ResourceState::UNDEFINED,
        bool isTransient                 = false
    )
        : type(type), initialState(state), transient(isTransient) {
      new (&renderTargetDesc) RenderTargetDesc(desc);
    }

    ResourceDesc(
        ::kst::core::ResourceType type,
        const BindlessTableDesc& desc,
        ::kst::core::ResourceState state = ::kst::core::ResourceState::UNDEFINED,
        bool isTransient                 = false
    )
        : type(type), initialState(state), transient(isTransient) {
      new (&bindlessTableDesc) BindlessTableDesc(desc);
    }

    auto getBufferDesc() const -> const BufferDesc& {
      assert(
          type == ::kst::core::ResourceType::BUFFER ||
          type == ::kst::core::ResourceType::INDEX_BUFFER ||
          type == ::kst::core::ResourceType::VERTEX_BUFFER ||
          type == ::kst::core::ResourceType::UNIFORM_BUFFER ||
          type == ::kst::core::ResourceType::STORAGE_BUFFER
      );
      return bufferDesc;
    }

    auto getTextureDesc() const -> const TextureDesc& {
      assert(type == ::kst::core::ResourceType::TEXTURE);
      return textureDesc;
    }

    auto getRenderTargetDesc() const -> const RenderTargetDesc& {
      assert(
          type == ::kst::core::ResourceType::RENDER_TARGET ||
          type == ::kst::core::ResourceType::DEPTH_STENCIL
      );
      return renderTargetDesc;
    }

    auto getBindlessTableDesc() const -> const BindlessTableDesc& {
      assert(type == ::kst::core::ResourceType::BINDLESS_TABLE);
      return bindlessTableDesc;
    }

    ~ResourceDesc() {
      switch (type) {
      case ::kst::core::ResourceType::BUFFER:
      case ::kst::core::ResourceType::VERTEX_BUFFER:
      case ::kst::core::ResourceType::INDEX_BUFFER:
      case ::kst::core::ResourceType::UNIFORM_BUFFER:
      case ::kst::core::ResourceType::STORAGE_BUFFER:
        bufferDesc.~BufferDesc();
        break;
      case ::kst::core::ResourceType::TEXTURE:
        textureDesc.~TextureDesc();
        break;
      case ::kst::core::ResourceType::RENDER_TARGET:
      case ::kst::core::ResourceType::DEPTH_STENCIL:
        renderTargetDesc.~RenderTargetDesc();
        break;
      case ::kst::core::ResourceType::BINDLESS_TABLE:
        bindlessTableDesc.~BindlessTableDesc();
        break;
      case ::kst::core::ResourceType::UNKNOWN:
      case ::kst::core::ResourceType::MESH:
      case ::kst::core::ResourceType::MATERIAL:
        break;
      }
    }
  };

  class RenderResource {
  public:
    RenderResource() = default;

    explicit RenderResource(const ResourceDesc& desc);

    RenderResource(
        ::kst::core::ResourceType type,
        resource::ResourceID rID,
        ::kst::core::ResourceState initialState = ::kst::core::ResourceState::GENERAL
    );

    RenderResource(::kst::core::ResourceType type, uint32_t bindlessIndex);

    auto getName() const -> const std::string&;
    void setName(std::string& name) { m_name = name; }

    auto getType() const -> ::kst::core::ResourceType { return m_type; }

    auto getState() const -> ::kst::core::ResourceState { return m_state; }
    void setState(::kst::core::ResourceState state) { m_state = state; }

    auto getResourceID() const -> resource::ResourceID { return m_resourceID; }
    void setResourceID(resource::ResourceID idx) { m_resourceID = idx; }

    auto isTransient() const -> bool { return m_transient; }
    void setTransient(bool transient) { m_transient = transient; }

    auto isBindless() const -> bool { return m_bindlessIndex.has_value(); }
    auto getBindlessIndex() const -> uint32_t {
      return m_bindlessIndex.has_value() ? m_bindlessIndex.value() : UINT32_MAX;
    }

    auto hasResourceDesc() const -> bool { return m_resourceDesc.has_value(); }
    auto getResourceDesc() const -> const ResourceDesc& { return m_resourceDesc.value(); }

    void setProducer(framegraph::RenderPass* pass) { m_producer = pass; }
    auto getProducer() const -> framegraph::RenderPass* { return m_producer; }

    void addConsumer(framegraph::RenderPass* pass);

    auto getConsumers() const -> const std::vector<framegraph::RenderPass*>& {
      return m_consumers;
    };

    void markUsed() { m_usedThisFrame = true; }
    auto isUsedThisFrame() const -> bool { return m_usedThisFrame; }
    void resetUsage() { m_usedThisFrame = false; }

  private:
    std::string m_name;
    ::kst::core::ResourceType m_type   = ::kst::core::ResourceType::BUFFER;
    ::kst::core::ResourceState m_state = ::kst::core::ResourceState::UNDEFINED;

    resource::ResourceID m_resourceID = {.index = (UINT32_MAX), .generation = 0};

    std::optional<uint32_t> m_bindlessIndex;

    std::optional<ResourceDesc> m_resourceDesc;

    bool m_transient     = false;
    bool m_usedThisFrame = false;

    framegraph::RenderPass* m_producer = nullptr;
    std::vector<framegraph::RenderPass*> m_consumers;
  };

} // namespace kst::renderer::resources
