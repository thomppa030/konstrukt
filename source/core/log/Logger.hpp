#pragma once

#include <cstdint>
#include <memory>
#include <source_location>
#include <string>
#include <string_view>

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace kst::core {

  enum class LogLevel : std::uint8_t {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    CRITICAL,
    OFF
  };

  struct LogContext {
    std::string_view file;
    std::string_view function;
    uint32_t line;

    constexpr LogContext(
        std::string_view file     = {},
        std::string_view function = {},
        uint32_t line             = 0
    )
        : file(file), function(function), line(line) {}

    constexpr LogContext(const std::source_location& location)
        : file(location.file_name()), function(location.function_name()), line(location.line()) {}
  };

  class Logger {
  public:
    static void init(
        const std::string& logFile = "konstrukt.log",
        size_t maxFileSize         = 5 * 1024 * 1024,
        int maxFiles               = 3
    );
    static void shutdown();

    static void setLevel(LogLevel level);
    static LogLevel getLevel();

    // Core engine logging with source location
    template <typename... Args>
    static void
    log(LogLevel level,
        std::string_view format,
        const Args&... args,
        const std::source_location& location = std::source_location::current()) {
      if (!sInitialized) {
        return;
      }

      auto& logger = sCoreLogger;

      // Convert to spdlog level
      spdlog::level::level_enum spdlogLevel;
      switch (level) {
      case LogLevel::TRACE:
        spdlogLevel = spdlog::level::trace;
        break;
      case LogLevel::DEBUG:
        spdlogLevel = spdlog::level::debug;
        break;
      case LogLevel::INFO:
        spdlogLevel = spdlog::level::info;
        break;
      case LogLevel::WARN:
        spdlogLevel = spdlog::level::warn;
        break;
      case LogLevel::ERROR:
        spdlogLevel = spdlog::level::err;
        break;
      case LogLevel::CRITICAL:
        spdlogLevel = spdlog::level::critical;
        break;
      default:
        spdlogLevel = spdlog::level::info;
        break;
      }

      // Only log if level is sufficient
      if (logger->should_log(spdlogLevel)) {
        const auto fileStr     = std::string(location.file_name());
        const auto fileNamePos = fileStr.find_last_of("/\\");
        const auto fileName =
            (fileNamePos == std::string::npos) ? fileStr : fileStr.substr(fileNamePos + 1);

        logger->log(
            spdlogLevel,
            "[{0}:{1}] {2}",
            fileName,
            location.line(),
            fmt::format(fmt::runtime(std::string(format)), args...)
        );
      }
    }

    // Specialized logging methods with source location
    template <typename... Args>
    static constexpr void trace(
        std::string_view format,
        const Args&... args,
        const std::source_location& location = std::source_location::current()
    ) {
      // Forward args explicitly
      log<Args...>(LogLevel::TRACE, format, args..., location);
    }

    template <typename... Args>
    static constexpr void debug(
        std::string_view format,
        const Args&... args,
        const std::source_location& location = std::source_location::current()
    ) {
      log<Args...>(LogLevel::DEBUG, format, args..., location);
    }

    template <typename... Args>
    static constexpr void info(
        std::string_view format,
        const Args&... args,
        const std::source_location& location = std::source_location::current()
    ) {
      log<Args...>(LogLevel::INFO, format, args..., location);
    }

    template <typename... Args>
    static constexpr void warn(
        std::string_view format,
        const Args&... args,
        const std::source_location& location = std::source_location::current()
    ) {
      log<Args...>(LogLevel::WARN, format, args..., location);
    }

    template <typename... Args>
    static constexpr void error(
        std::string_view format,
        const Args&... args,
        const std::source_location& location = std::source_location::current()
    ) {
      log<Args...>(LogLevel::ERROR, format, args..., location);
    }

    template <typename... Args>
    static constexpr void critical(
        std::string_view format,
        const Args&... args,
        const std::source_location& location = std::source_location::current()
    ) {
      log<Args...>(LogLevel::CRITICAL, format, args..., location);
    }

    // Client application logging
    template <typename... Args>
    static constexpr void appLog(
        LogLevel level,
        std::string_view format,
        const Args&... args,
        const std::source_location& location = std::source_location::current()
    ) {
      if (!sInitialized) {
        return;
      }

      auto& logger = sClientLogger;

      // Convert to spdlog level
      spdlog::level::level_enum spdlogLevel;
      switch (level) {
      case LogLevel::TRACE:
        spdlogLevel = spdlog::level::trace;
        break;
      case LogLevel::DEBUG:
        spdlogLevel = spdlog::level::debug;
        break;
      case LogLevel::INFO:
        spdlogLevel = spdlog::level::info;
        break;
      case LogLevel::WARN:
        spdlogLevel = spdlog::level::warn;
        break;
      case LogLevel::ERROR:
        spdlogLevel = spdlog::level::err;
        break;
      case LogLevel::CRITICAL:
        spdlogLevel = spdlog::level::critical;
        break;
      default:
        spdlogLevel = spdlog::level::info;
        break;
      }

      // Only log if level is sufficient
      if (logger->should_log(spdlogLevel)) {
        const auto fileStr     = std::string(location.file_name());
        const auto fileNamePos = fileStr.find_last_of("/\\");
        const auto fileName =
            (fileNamePos == std::string::npos) ? fileStr : fileStr.substr(fileNamePos + 1);

        logger->log(
            spdlogLevel,
            "[{0}:{1}] {2}",
            fileName,
            location.line(),
            fmt::format(fmt::runtime(std::string(format)), args...)
        );
      }
    }

    void trace(const std::string& message);

    void debug(const std::string& message);

    void info(const std::string& message);

    void warn(const std::string& message);

    void error(const std::string& message);

    void critical(const std::string& message);

    template <typename... Args>
    static constexpr void appTrace(
        std::string_view format,
        const Args&... args,
        const std::source_location& location = std::source_location::current()
    ) {
      appLog<Args...>(LogLevel::TRACE, format, args..., location);
    }

    template <typename... Args>
    static constexpr void appDebug(
        std::string_view format,
        const Args&... args,
        const std::source_location& location = std::source_location::current()
    ) {
      appLog<Args...>(LogLevel::DEBUG, format, args..., location);
    }

    template <typename... Args>
    static constexpr void appInfo(
        std::string_view format,
        const Args&... args,
        const std::source_location& location = std::source_location::current()
    ) {
      appLog<Args...>(LogLevel::INFO, format, args..., location);
    }

    template <typename... Args>
    static constexpr void appWarn(
        std::string_view format,
        const Args&... args,
        const std::source_location& location = std::source_location::current()
    ) {
      appLog<Args...>(LogLevel::WARN, format, args..., location);
    }

    template <typename... Args>
    static constexpr void appError(
        std::string_view format,
        const Args&... args,
        const std::source_location& location = std::source_location::current()
    ) {
      appLog<Args...>(LogLevel::ERROR, format, args..., location);
    }

    template <typename... Args>
    static constexpr void appCritical(
        std::string_view format,
        const Args&... args,
        const std::source_location& location = std::source_location::current()
    ) {
      appLog<Args...>(LogLevel::CRITICAL, format, args..., location);
    }

    // Get raw loggers (if needed for advanced usage)
    static auto getCoreLogger() -> std::shared_ptr<spdlog::logger>& { return sCoreLogger; }
    static auto getClientLogger() -> std::shared_ptr<spdlog::logger>& { return sClientLogger; }

  private:
    static std::shared_ptr<spdlog::logger> sCoreLogger;
    static std::shared_ptr<spdlog::logger> sClientLogger;
    static bool sInitialized;
  };
} // namespace kst::core
