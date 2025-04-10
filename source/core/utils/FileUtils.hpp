#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <boost/range/algorithm/transform.hpp>

namespace kst::core::utils {

  enum class KstFileType : std::uint8_t {
    UNKNOWN,
    GLTF,
    OBJ,
    JPEG,
    PNG,
    GIF,
    SVG,
  };

  class FileTypeDetector {
  public:
    FileTypeDetector();
    ~FileTypeDetector();

    FileTypeDetector(const FileTypeDetector&)                    = default;
    FileTypeDetector(FileTypeDetector&&)                         = delete;
    auto operator=(const FileTypeDetector&) -> FileTypeDetector& = default;
    auto operator=(FileTypeDetector&&) -> FileTypeDetector&      = delete;

    static auto getFileType(std::string& path) -> KstFileType {
      auto type = detectByExtension(path);

      if (type == KstFileType::UNKNOWN) {
        type = detectBySignature(path);
      }
      return type;
    }

  private:
    static auto detectByExtension(std::string& path) -> KstFileType {
      std::filesystem::path filePath(path);
      std::string extension = filePath.extension().string();

      boost::range::transform(extension, extension.begin(), [](unsigned char ext) {
        return std::tolower(ext);
      });

      if (extension == ".gltf" || extension == ".glb") {
        return KstFileType::GLTF;
      }
      // TODO: implement more Types
      return KstFileType::UNKNOWN;
    };

    static auto detectBySignature(std::string& path) -> KstFileType {
      std::ifstream file(path, std::ios::binary);
      if (!file) {
        return KstFileType::UNKNOWN;
      }

      std::vector<uint8_t> header(12, 0);
      file.read(reinterpret_cast<char*>(header.data()), header.size());

      // TODO: implement more Types
      return KstFileType::UNKNOWN;
    }
  };
} // namespace kst::core::utils
