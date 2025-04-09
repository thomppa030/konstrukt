#pragma once

#include <string>

#include "resources/ResourceID.hpp"
namespace kst::renderer::resource {
  class GLTFLoader {
  public:
    GLTFLoader();
    ~GLTFLoader();

    auto loadFile(std::string& path) -> ResourceID;

  private:
  };
} // namespace kst::renderer::resource
