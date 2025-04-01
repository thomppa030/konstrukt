#include "ResourceRegistry.hpp"

#include <atomic>
#include <cstdint>

#include "ResourceID.hpp"
#include "core/log/Logger.hpp"

namespace {
  // Static counter for generating unique resource IDs
  std::atomic<uint32_t> g_resourceIdCounter = 0;
} // namespace

namespace kst::renderer::resources {

  auto ResourceRegistry::registerResource(::kst::core::ResourceType type) -> resource::ResourceID {
    // Generate a new resource ID
    const uint32_t index = g_resourceIdCounter.fetch_add(1);
    const resource::ResourceID resourceId{
        .index      = index,
        .generation = 1
    }; // Start with generation 1

    // Record the resource type
    m_resourceTypes[resourceId] = type;

    kst::core::Logger::info<uint32_t, int>(
        "Registered resource with ID {} of type {}",
        resourceId.index,
        static_cast<int>(type)
    );

    return resourceId;
  }

  auto ResourceRegistry::registerBuffer(resource::ResourceID rID) -> uint32_t {
    // For buffers, we don't store any data yet, just record the ID
    m_bufferIndices[rID] = m_bufferIndices.size();
    m_resourceTypes[rID] = ::kst::core::ResourceType::BUFFER;

    kst::core::Logger::info<uint32_t>("Registered buffer with ID {}", rID.index);
    return m_bufferIndices[rID];
  }

  auto ResourceRegistry::registerMesh(resource::ResourceID rID, const MeshData& data) -> uint32_t {
    m_meshed.push_back(data);
    auto index           = static_cast<uint32_t>(m_meshed.size() - 1);
    m_meshIndices[rID]   = index;
    m_resourceTypes[rID] = ::kst::core::ResourceType::MESH;

    kst::core::Logger::info<uint32_t>("Registered mesh with ID {}", rID.index);
    return index;
  }

  auto ResourceRegistry::registerMaterial(resource::ResourceID rID, const MaterialData& data)
      -> uint32_t {
    m_materials.push_back(data);
    auto index             = static_cast<uint32_t>(m_materials.size() - 1);
    m_materialIndices[rID] = index;
    m_resourceTypes[rID]   = ::kst::core::ResourceType::MATERIAL;

    kst::core::Logger::info<uint32_t>("Registered material with ID {}", rID.index);
    return index;
  }

  auto ResourceRegistry::registerTexture(resource::ResourceID rID, const TextureData& data)
      -> uint32_t {
    m_textures.push_back(data);
    auto index            = static_cast<uint32_t>(m_textures.size() - 1);
    m_textureIndices[rID] = index;
    m_resourceTypes[rID]  = ::kst::core::ResourceType::TEXTURE;

    kst::core::Logger::info<uint32_t>("Registered texture with ID {}", rID.index);
    return index;
  }

  auto ResourceRegistry::getIndexForMesh(resource::ResourceID rID) -> uint32_t {
    auto iter = m_meshIndices.find(rID);
    if (iter != m_meshIndices.end()) {
      return iter->second;
    }

    kst::core::Logger::warn<uint32_t>("Mesh with ID {} not found", rID.index);
    return UINT32_MAX;
  }

  auto ResourceRegistry::getIndexForMaterial(resource::ResourceID rID) -> uint32_t {
    auto iter = m_materialIndices.find(rID);
    if (iter != m_materialIndices.end()) {
      return iter->second;
    }

    kst::core::Logger::warn<uint32_t>("Material with ID {} not found", rID.index);
    return UINT32_MAX;
  }

  auto ResourceRegistry::getIndexForTexture(resource::ResourceID rID) -> uint32_t {
    auto iter = m_textureIndices.find(rID);
    if (iter != m_textureIndices.end()) {
      return iter->second;
    }

    kst::core::Logger::warn<uint32_t>("Texture with ID {} not found", rID.index);
    return UINT32_MAX;
  }

  void ResourceRegistry::updateDescriptorTables() {
    // This is a placeholder implementation
    kst::core::Logger::debug("Updating descriptor tables");
  }

} // namespace kst::renderer::resources
