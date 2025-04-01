#include "ResourceManager.hpp"

#include "ResourceID.hpp"
#include "ResourceRegistry.hpp"
#include "core/log/Logger.hpp"
#include "renderer/core/GraphicsContext.hpp"

namespace kst::renderer::resources {

  ResourceManager::ResourceManager(core::GraphicsContext& context, ResourceRegistry& registry)
      : m_context(context), m_registry(registry) {
    kst::core::Logger::info<>("Resource manager initialized");
  }

  auto ResourceManager::createBuffer(
      const void* data [[maybe_unused]],
      std::size_t size,
      ::kst::core::BufferUsageFlags usage [[maybe_unused]]
  ) -> resource::ResourceID {
    kst::core::Logger::info<size_t>("Creating buffer of size {} bytes", size);

    // Create buffer description
    BufferDesc bufferDesc;
    bufferDesc.size  = size;
    bufferDesc.usage = usage;

    // Register the buffer in the resource registry
    auto resourceID = m_registry.registerResource(::kst::core::ResourceType::BUFFER);
    m_registry.registerBuffer(resourceID);

    // Store the buffer description for later use
    ResourceDesc resourceDesc(::kst::core::ResourceType::BUFFER, bufferDesc);
    m_resourceDescriptions[resourceID] = resourceDesc;

    // TODO: Implement actual GPU buffer allocation through the graphics context
    // m_context.createBuffer(resourceID, size, usage, data);

    return resourceID;
  }

  auto ResourceManager::createTexture(const TextureDesc& desc) -> resource::ResourceID {
    kst::core::Logger::info<uint32_t, uint32_t>(
        "Creating texture of size {}x{}",
        desc.width,
        desc.height
    );

    // Create TextureData from TextureDesc for the registry
    TextureData textureData;
    textureData.width       = desc.width;
    textureData.height      = desc.height;
    textureData.depth       = desc.depth;
    textureData.mipLevels   = desc.mipLevels;
    textureData.arrayLayers = desc.arrayLevels;
    textureData.format      = desc.format;
    textureData.cubeMap     = desc.cubeMap;

    // Register the texture in the resource registry
    auto resourceID = m_registry.registerResource(::kst::core::ResourceType::TEXTURE);
    m_registry.registerTexture(resourceID, textureData);

    // Store the texture description for later use
    ResourceDesc resourceDesc(::kst::core::ResourceType::TEXTURE, desc);
    m_resourceDescriptions[resourceID] = resourceDesc;

    // TODO: Implement actual GPU texture allocation through the graphics context
    // m_context.createTexture(resourceID, desc);

    return resourceID;
  }

  auto ResourceManager::getResourceDesc(resource::ResourceID resourceId) const
      -> const ResourceDesc* {
    auto iter = m_resourceDescriptions.find(resourceId);
    if (iter != m_resourceDescriptions.end()) {
      return &(iter->second);
    }
    return nullptr;
  }

} // namespace kst::renderer::resources
