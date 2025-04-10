#include <string>

#include <glm/fwd.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "resources/ResourceID.hpp"

namespace kst::renderer::resources {
  struct ModelNode {
    std::string name;
    glm::mat4 transform = glm::mat4(1.0F);
    resource::ResourceID meshID;
    resource::ResourceID materialID;
    std::vector<ModelNode> children;
  };

  struct ModelData {
    std::string name;
    ModelNode rootNode;
    std::vector<resource::ResourceID> meshes;
    std::vector<resource::ResourceID> materials;
    std::vector<resource::ResourceID> textures;

    bool hasAnimations = false;
  };

  struct ModelLoadingOptions {
    bool generateTangents = true;
    bool optimizeMeshes   = true;
    bool flipUVs          = true;
    bool loadMaterials    = true;
    bool loadTextures     = true;
    bool loadAnimations   = false;
    float scaleFactor     = 1.0F;
  };
} // namespace kst::renderer::resources
