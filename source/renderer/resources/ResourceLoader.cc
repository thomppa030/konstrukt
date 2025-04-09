#include <string>

#include "resources/ResourceID.hpp"
#include "utils/FileUtils.hpp"

namespace kst::renderer::resource {

  static auto loadResource(std::string& path) -> ResourceID {
    core::utils::KstFileType type = core::utils::FileTypeDetector::getFileType(path);
    switch (type) {
    case kst::core::utils::KstFileType::GLTF:
      return GLTFLoader::loadfile(path);
      break;
    default:
      return ResourceID::invalid();
    }
    return ResourceID::invalid();
  };
} // namespace kst::renderer::resource
