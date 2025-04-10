
#include "resources/ModelLoader.hpp"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <utility>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "log/Logger.hpp"
#include "resources/ResourceID.hpp"
#include "resources/ResourceManager.hpp"

namespace kst::renderer::resources {

  ModelLoader::ModelLoader(
      std::shared_ptr<ResourceManager> resourceManager,
      std::shared_ptr<ResourceRegistry> resourceRegistry
  )
      : m_resourceManager(std::move(resourceManager)),
        m_resourceRegistry(std::move(resourceRegistry)) {
    kst::core::Logger::info("Model Loader initialized!");
  }

  auto ModelLoader::loadModel(const std::string& filePath, const ModelLoadingOptions& options)
      -> resource::ResourceID {
    kst::core::Logger::info<const std::string&>("Loading model: {}", filePath);

    if (!std::filesystem::exists(filePath)) {
      kst::core::Logger::error<const std::string&>("Model file not found: {}", filePath);
      return resource::ResourceID::invalid();
    }

    unsigned int flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                         aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices;

    if (options.generateTangents) {
      flags |= aiProcess_CalcTangentSpace;
    }

    if (options.optimizeMeshes) {
      flags |= aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph;
    }

    if (options.flipUVs) {
      flags |= aiProcess_FlipUVs;
    }

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filePath, flags);

    if ((scene == nullptr) || ((scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0U) ||
        (scene->mRootNode == nullptr)) {
      kst::core::Logger::error<const std::string&, const std::string&>(
          "Failed to load model: {}, Assimp error: {}",
          filePath,
          importer.GetErrorString()
      );
      return resource::ResourceID::invalid();
    }

    std::string basePath = filePath.substr(0, filePath.find_last_of('/') + 1);

    ModelData modelData = processAssimpScene(scene, basePath, options);
    modelData.name      = std::filesystem::path(filePath).filename().string();

    resource::ResourceID modelID = createModelResource(modelData);

    kst::core::Logger::info<const std::string&, uint32_t, uint32_t>(
        "Model Loaded successfully: {} {{} meshes, {} materials}",
        modelData.name,
        modelData.meshes.size(),
        modelData.materials.size()
    );

    return modelID;
  }

  auto ModelLoader::processAssimpScene(
      const aiScene* scene,
      const std::string& basePath,
      const ModelLoadingOptions& options
  ) -> ModelData {
    ModelData modelData;
    modelData.name = scene->mName.C_Str();

    if (modelData.name.empty()) {
      modelData.name = "Unnamed Model";
    }

    std::vector<resource::ResourceID> materialIDs;

    if (options.loadMaterials) {
      materialIDs.reserve(scene->mNumMaterials);

      for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        MaterialData materialData       = processAssimpMaterial(scene->mMaterials[i], basePath);
        resource::ResourceID materialID = createMaterialResource(materialData);
        materialIDs.push_back(materialID);
      }
      modelData.materials = std::move(materialIDs);
    }

    std::vector<resource::ResourceID> meshIDs;
    meshIDs.reserve(scene->mNumMeshes);

    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
      aiMesh* currentMesh = scene->mMeshes[i];
      MeshData meshData   = processAssimpMesh(currentMesh, scene);

      if (options.loadMaterials && currentMesh->mMaterialIndex >= 0 &&
          currentMesh->mMaterialIndex < materialIDs.size()) {
        meshData.defaultMaterial = materialIDs[currentMesh->mMaterialIndex];
      }

      resource::ResourceID meshID = createMeshResource(meshData);
      meshIDs.push_back(meshID);
    }
    modelData.meshes = std::move(meshIDs);

    modelData.rootNode = processAssimpNode(scene->mRootNode, scene, meshIDs);

    if (options.loadAnimations && scene->HasAnimations()) {
      modelData.hasAnimations = true;

      kst::core::Logger::info<uint32_t>(
          "Model has {} animations, but Animations are currently not supported.",
          scene->mNumAnimations
      );
    }

    return modelData;
  }
} // namespace kst::renderer::resources
