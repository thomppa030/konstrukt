#pragma once

#include <memory>
#include <string>
#include <vector>

#include <assimp/material.h>
#include <assimp/scene.h>

#include "ModelTypes.hpp"
#include "ResourceManager.hpp"
#include "ResourceRegistry.hpp"
#include "resources/ResourceID.hpp"

namespace kst::renderer::resources {
  class ModelLoader {
  public:
    ModelLoader(
        std::shared_ptr<ResourceManager> resourceManager,
        std::shared_ptr<ResourceRegistry> resourceRegistry
    );
    ~ModelLoader() = default;

    ModelLoader(const ModelLoader&)                    = default;
    ModelLoader(ModelLoader&&)                         = delete;
    auto operator=(const ModelLoader&) -> ModelLoader& = delete;
    auto operator=(ModelLoader&&) -> ModelLoader&      = delete;

    auto loadModel(const std::string& filePath, const ModelLoadingOptions& options = {})
        -> resource::ResourceID;

  private:
    std::shared_ptr<ResourceManager> m_resourceManager;
    std::shared_ptr<ResourceRegistry> m_resourceRegistry;

    auto processAssimpScene(
        const aiScene* scene,
        const std::string& basePath,
        const ModelLoadingOptions& options
    ) -> ModelData;

    auto processAssimpMesh(const aiMesh* mesh, const aiScene* scene) -> MeshData;

    auto processAssimpMaterial(const aiMaterial* material, const std::string& basePath)
        -> MaterialData;

    auto processAssimpTexture(
        const aiMaterial* material,
        aiTextureType type,
        const std::string& basePath
    ) -> resource::ResourceID;

    auto processAssimpNode(
        aiNode* node,
        const aiScene* scene,
        const std::vector<resource::ResourceID>& meshIDs
    ) -> ModelNode;

    auto createModelResource(const ModelData& modelData) -> resource::ResourceID;
    auto createMaterialResource(const MaterialData& materialData) -> resource::ResourceID;
    auto createMeshResource(const MeshData& meshData) -> resource::ResourceID;
  };
} // namespace kst::renderer::resources
