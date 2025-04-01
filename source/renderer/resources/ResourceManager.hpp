#pragma once

#include <cstddef>

#include "RenderResource.hpp"
#include "ResourceRegistry.hpp"
#include "renderer/core/GraphicsContext.hpp"
#include "renderer/core/GraphicsType.hpp"

namespace kst::renderer::resources {

  class ResourceManager {
  public:
    ResourceManager(core::GraphicsContext& context, ResourceRegistry& registry);
    ~ResourceManager() = default;

    ResourceManager(const ResourceManager&)                    = delete;
    ResourceManager(ResourceManager&&)                         = delete;
    auto operator=(const ResourceManager&) -> ResourceManager& = delete;
    auto operator=(ResourceManager&&) -> ResourceManager&      = delete;

    auto createBuffer(const void* data, std::size_t size, ::kst::core::BufferUsageFlags usage)
        -> resource::ResourceID;

    auto createTexture(const TextureDesc& desc) -> resource::ResourceID;
    
    auto getResourceDesc(resource::ResourceID resourceId) const -> const ResourceDesc*;

  private:
    core::GraphicsContext& m_context;
    ResourceRegistry& m_registry;
    std::unordered_map<resource::ResourceID, ResourceDesc> m_resourceDescriptions;
  };
} // namespace kst::renderer::resources
