#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

#include "Result.hpp"
namespace kst::core {
  class FileSystem {
  public:
    enum class PathSeparator : std::uint8_t {
      NATIVE,
      WINDOWS,
      UNIX
    };

    struct FileTimeInfo {
      std::chrono::system_clock::time_point lastAccessTime;
      std::chrono::system_clock::time_point lastModificationTime;
      std::chrono::system_clock::time_point creationTime;
    };

    struct FileInfo {
      std::string path;
      uintmax_t size;
      bool isDirectory;
      bool isRegularFile;
      bool isSymLink;
      bool isHidden;
      FileTimeInfo timeInfo;
    };

    /*
     *  @brief Initialize the filesystem handler
     *  @param applicationName Name of the application
     *  @return Result
     */
    static auto initialize(const std::string& applicationName) -> Result<void>;

    /*
     * @brief shutdown the filesystem and clean up resources
     */
    static void shutdown();

    static auto toAbsolutePath(const std::string& path) -> Result<std::string>;

    static auto
    normalizePath(const std::string& path, PathSeparator separator = PathSeparator::NATIVE)
        -> std::string;

    static auto joinPaths(const std::vector<std::string>& path) -> std::string;

    static auto joinPath(const std::string& path1, const std::string& path2) -> std::string;

    static auto getParentPath(const std::string& path) -> std::string;

    static auto getFileName(const std::string& path) -> std::string;

    static auto getFileExtension(const std::string& path) -> std::string;

    static auto getFileStem(const std::string& path) -> std::string;

    static auto exists(const std::string& path) -> bool;

    static auto isDirectory(const std::string& path) -> bool;

    static auto isFile(const std::string& path) -> bool;

    static auto isSymlink(const std::string& path) -> bool;

    static auto isHidden(const std::string& path) -> bool;

    static auto createDirectory(const std::string& path, bool recursive = false) -> Result<void>;

    static auto removeFile(const std::string& path) -> Result<void>;

    static auto removeDirectory(const std::string& path, bool recursive = false) -> Result<void>;

    static auto
    copyFile(const std::string& source, const std::string& destination, bool recursive = false)
        -> Result<void>;

    static auto moveFile(const std::string& source, const std::string& destination) -> Result<void>;

    static auto getSize(const std::string& path) -> Result<uintmax_t>;

    static auto getFileTimes(const std::string& path) -> Result<FileTimeInfo>;

    static auto getFileInfo(const std::string& path) -> Result<FileInfo>;

    static auto listDirectory(const std::string& path, bool recursive)
        -> Result<std::vector<std::string>>;

    static auto listDirectoryInfo(const std::string& path, bool recursive)
        -> Result<std::vector<FileInfo>>;

    static auto readTextFile(const std::string& path) -> Result<std::string>;

    static auto readBinaryFile(const std::string& path) -> Result<std::vector<uint8_t>>;

    static auto writeTextFile(const std::string& path, const std::string& content, bool append)
        -> Result<void>;

    static auto
    writeBinaryFile(const std::string& path, const std::vector<uint8_t>& data, bool append)
        -> Result<void>;

    static auto getCurrentDirectory() -> Result<std::string>;

    static auto setCurrentDirectory(const std::string& path) -> Result<std::string>;

    static auto getExecutablePath() -> Result<std::string>;

    static auto getApplicationDirectory() -> Result<std::string>;

    static auto getHomeDirectory() -> Result<std::string>;

    static auto getAppDataDirectory() -> Result<std::string>;

    static auto getTempDirectory() -> Result<std::string>;

    static auto createTempFile(const std::string& prefix = "", const std::string& extension = "")
        -> Result<std::string>;

    static auto createTempDirectory(const std::string& prefix = "") -> Result<std::string>;

    static auto watchDirectory(
        const std::string& path,
        std::function<void(const std::string&, bool)> callback,
        bool recursive
    ) -> Result<int>;

    static auto stopWatching(int watchID) -> Result<void>;

    FileSystem() = delete;

  private:
    static std::string sApplicationName;
    static std::string sapplicationName;
    static std::optional<std::string> sExecutablePath;
    static bool sInitialized;

    static auto getNativeSeparator() -> char;
    static auto isAbsolutePath(const std::string& path) -> bool;
    static auto sanitizePath(const std::string& path) -> std::string;
  };
} // namespace kst::core
