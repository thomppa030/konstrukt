#pragma once

#include <string>

#include "resources/ResourceID.hpp"

namespace kst::renderer::resource {
  class ResourceLoader {
  public:
    ResourceLoader()  = default;
    ~ResourceLoader() = default;

    ResourceLoader(const ResourceLoader&)                    = default;
    ResourceLoader(ResourceLoader&&)                         = delete;
    auto operator=(const ResourceLoader&) -> ResourceLoader& = default;
    auto operator=(ResourceLoader&&) -> ResourceLoader&      = delete;

    auto getFilePath() -> std::string&;

    auto loadResource(std::string& path) -> ResourceID;
  };
} // namespace kst::renderer::resource
