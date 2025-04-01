#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>

#include "ResourceID.hpp"
#include "renderer/core/GraphicsType.hpp"

namespace kst::renderer::resources {

  struct MeshData {
    std::vector<float> positions;
    std::vector<float> normals;
    std::vector<float> texCoords;
    std::vector<float> tangents;
    std::vector<float> colors;

    std::vector<float> boneWeights;
    std::vector<uint32_t> boneIndices;

    std::vector<uint32_t> indices;

    uint32_t vertexCount = 0;
    uint32_t indexCount  = 0;
    bool hasIndices      = false;

    // TODO implement Collision, when physics are tackled
    /*
      AABB boundingBox;
      float boundingSphereRadius = 0.0F;
    */

    resource::ResourceID defaultMaterial;
  };

  constexpr float DEFAULT_ROUGHNESS = 0.5F;
  constexpr float DEFAULT_METALLIC  = 0.0F;
  constexpr float DEFAULT_SPECULAR  = 0.5F;
  constexpr float DEFAULT_IOR       = 1.45F;

  constexpr float DEFAULT_HEIGHTSCALE = 0.05F;
  constexpr float DEFAULT_ALPHACUTOFF = 0.5F;

  constexpr int DEFAULT_RENDERQUEUE_SORTING_ORDER = 2000;

  struct MaterialData {
    glm::vec4 baseColor = {1.0F, 1.0F, 1.0F, 1.0F};

    float roughness = DEFAULT_ROUGHNESS;
    float metallic  = DEFAULT_METALLIC;
    float specular  = DEFAULT_SPECULAR;
    float ior       = DEFAULT_IOR;

    float emission          = 0.0F;
    glm::vec3 emissionColor = {1.0F, 1.0F, 1.0F};

    resource::ResourceID albedoMap;
    resource::ResourceID normalMap;
    resource::ResourceID roughnessMap;
    resource::ResourceID metallicMap;
    resource::ResourceID aoMap;
    resource::ResourceID emissionMap;
    resource::ResourceID heightMap;

    bool useAlbedoMap    = false;
    bool useNormalMap    = false;
    bool useRoughnessMap = false;
    bool useMetallicMap  = false;
    bool useAOMap        = false;
    bool useEmissionMap  = false;
    bool useHeightMap    = false;

    float normalMapIntensity = 1.0F;
    float heightScale        = DEFAULT_HEIGHTSCALE;
    bool alphaBlend          = false;
    float alphaCutoff        = DEFAULT_ALPHACUTOFF;

    resource::ResourceID shaderProgram;
    uint32_t renderQueue = DEFAULT_RENDERQUEUE_SORTING_ORDER;
  };

  struct TextureData {
    uint32_t width       = 0;
    uint32_t height      = 0;
    uint32_t depth       = 1;
    uint32_t mipLevels   = 1;
    uint32_t arrayLayers = 1;

    ::kst::core::Format format = ::kst::core::Format::UNKNOWN;
    std::vector<uint8_t> pixels;

    // TODO implement Enums for FilterMode and AdressMode
    /* FilterMode minFilter = FilterMode::Linear;
    FilterMode magFilter = FilterMode::Linear;
    AdressMode addressU  = AdressMode::Repeat;
    AdressMode addressV  = AdressMode::Repeat;
    AdressMode addressW  = AdressMode::Repeat; */

    bool generateMipMaps = true;
    bool sRGB            = false;
    bool cubeMap         = false;
    bool compressData    = true;

    std::string sourcePath;
  };

  class ResourceRegistry {
  public:
    auto registerResource(::kst::core::ResourceType type) -> resource::ResourceID;
    auto registerMesh(resource::ResourceID rID, const MeshData& data) -> uint32_t;
    auto registerMaterial(resource::ResourceID rID, const MaterialData& data) -> uint32_t;
    auto registerTexture(resource::ResourceID rID, const TextureData& data) -> uint32_t;
    auto registerBuffer(resource::ResourceID rID) -> uint32_t;

    auto getIndexForMesh(resource::ResourceID rID) -> uint32_t;
    auto getIndexForMaterial(resource::ResourceID rID) -> uint32_t;
    auto getIndexForTexture(resource::ResourceID rID) -> uint32_t;

    /**
     * @brief Determine the resource type associated with a given ResourceID
     *
     * This function checks if the provided ResourceID is registered as a mesh,
     * material, or texture, and returns the appropriate ResourceType.
     *
     * @param resource The ResourceID to check
     * @return core::ResourceType The type of the resource, or ResourceType::UNKNOWN if not found
     */
    auto getResourceType(resource::ResourceID resource) const -> ::kst::core::ResourceType {
      // Check in our resource type map first
      auto iter = m_resourceTypes.find(resource);
      if (iter != m_resourceTypes.end()) {
        return iter->second;
      }

      // Legacy checks for backward compatibility
      if (m_meshIndices.contains(resource)) {
        return ::kst::core::ResourceType::MESH;
      }

      if (m_materialIndices.contains(resource)) {
        return ::kst::core::ResourceType::MATERIAL;
      }

      if (m_textureIndices.contains(resource)) {
        return ::kst::core::ResourceType::TEXTURE;
      }

      // Resource not found in any registry
      return ::kst::core::ResourceType::UNKNOWN;
    }

    void updateDescriptorTables();

  private:
    std::vector<MeshData> m_meshed;
    std::vector<MaterialData> m_materials;
    std::vector<TextureData> m_textures;

    std::unordered_map<resource::ResourceID, uint32_t> m_meshIndices;
    std::unordered_map<resource::ResourceID, uint32_t> m_materialIndices;
    std::unordered_map<resource::ResourceID, uint32_t> m_textureIndices;
    std::unordered_map<resource::ResourceID, uint32_t> m_bufferIndices;

    // For mapping ResourceIDs to their types
    std::unordered_map<resource::ResourceID, ::kst::core::ResourceType> m_resourceTypes;
  };

} // namespace kst::renderer::resources
