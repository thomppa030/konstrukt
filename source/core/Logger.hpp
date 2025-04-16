#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

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

  struct SourceLocation {
    constexpr SourceLocation(const char* file = "", const char* function = "", int line = 0)
        : file(file), function(function), line(line) {}

    const char* file;
    const char* function;
    int line;
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
    static auto getLevel() -> LogLevel;

    // Core engine logging - simple string
    static void logCore(LogLevel level, const SourceLocation& location, const char* message) {
      if (!sInitialized) {
        return;
      }
      logInternal(level, sCoreLogger, location, message);
    }

    // Core engine logging - with formatting
    template <typename... Args>
    static void logCore(
        LogLevel level,
        const SourceLocation& location,
        spdlog::format_string_t<Args...> fmt,
        Args&&... args
    ) {
      if (!sInitialized) {
        return;
      }
      logInternal(level, sCoreLogger, location, fmt, std::forward<Args>(args)...);
    }

    // Application logging - simple string
    static void logApp(LogLevel level, const SourceLocation& location, const char* message) {
      if (!sInitialized) {
        return;
      }
      logInternal(level, sClientLogger, location, message);
    }

    // Application logging - with formatting
    template <typename... Args>
    static void logApp(
        LogLevel level,
        const SourceLocation& location,
        spdlog::format_string_t<Args...> fmt,
        Args&&... args
    ) {
      if (!sInitialized) {
        return;
      }
      logInternal(level, sClientLogger, location, fmt, std::forward<Args>(args)...);
    }

    // Get raw loggers (if needed for advanced usage)
    static auto getCoreLogger() -> std::shared_ptr<spdlog::logger>& { return sCoreLogger; }
    static auto getClientLogger() -> std::shared_ptr<spdlog::logger>& { return sClientLogger; }

  private:
    // Internal logging for simple strings
    static void logInternal(
        LogLevel level,
        std::shared_ptr<spdlog::logger>& logger,
        const SourceLocation& location,
        const char* message
    ) {
      spdlog::level::level_enum spdlogLevel = toSpdLogLevel(level);

      // Only log if level is sufficient
      if (logger->should_log(spdlogLevel)) {
        // Extract just the filename from the path
        const char* fileName  = location.file;
        const char* lastSlash = nullptr;

        for (const char* ch = fileName; *ch != 0; ++ch) {
          if (*ch == '/' || *ch == '\\') {
            lastSlash = ch;
          }
        }

        if (lastSlash != nullptr) {
          fileName = lastSlash + 1;
        }

        logger->log(spdlogLevel, "[{}:{}] {}", fileName, location.line, message);
      }
    }

    // Internal logging with formatting
    template <typename... Args>
    static void logInternal(
        LogLevel level,
        std::shared_ptr<spdlog::logger>& logger,
        const SourceLocation& location,
        spdlog::format_string_t<Args...> fmt,
        Args&&... args
    ) {
      spdlog::level::level_enum spdlogLevel = toSpdLogLevel(level);

      // Only log if level is sufficient
      if (logger->should_log(spdlogLevel)) {
        // Extract just the filename from the path
        const char* fileName  = location.file;
        const char* lastSlash = nullptr;

        for (const char* ch = fileName; *ch; ++ch) {
          if (*ch == '/' || *ch == '\\') {
            lastSlash = ch;
          }
        }

        if (lastSlash) {
          fileName = lastSlash + 1;
        }

        logger->log(
            spdlogLevel,
            "[{}:{}] {}",
            fileName,
            location.line,
            fmt::format(fmt, std::forward<Args>(args)...)
        );
      }
    }

    static auto toSpdLogLevel(LogLevel level) -> spdlog::level::level_enum {
      switch (level) {
      case LogLevel::TRACE:
        return spdlog::level::trace;
      case LogLevel::DEBUG:
        return spdlog::level::debug;
      case LogLevel::INFO:
        return spdlog::level::info;
      case LogLevel::WARN:
        return spdlog::level::warn;
      case LogLevel::ERROR:
        return spdlog::level::err;
      case LogLevel::CRITICAL:
        return spdlog::level::critical;
      case LogLevel::OFF:
        return spdlog::level::off;
      default:
        return spdlog::level::info;
      }
    }

    static std::shared_ptr<spdlog::logger> sCoreLogger;
    static std::shared_ptr<spdlog::logger> sClientLogger;
    static bool sInitialized;
  };

} // namespace kst::core

// Macro for creating a source location
#define KST_LOCATION ::kst::core::SourceLocation(__FILE__, __FUNCTION__, __LINE__)

// Core log macros
#define KST_CORE_TRACE(...) \
  ::kst::core::Logger::logCore(::kst::core::LogLevel::TRACE, KST_LOCATION, __VA_ARGS__)
#define KST_CORE_DEBUG(...) \
  ::kst::core::Logger::logCore(::kst::core::LogLevel::DEBUG, KST_LOCATION, __VA_ARGS__)
#define KST_CORE_INFO(...) \
  ::kst::core::Logger::logCore(::kst::core::LogLevel::INFO, KST_LOCATION, __VA_ARGS__)
#define KST_CORE_WARN(...) \
  ::kst::core::Logger::logCore(::kst::core::LogLevel::WARN, KST_LOCATION, __VA_ARGS__)
#define KST_CORE_ERROR(...) \
  ::kst::core::Logger::logCore(::kst::core::LogLevel::ERROR, KST_LOCATION, __VA_ARGS__)
#define KST_CORE_CRITICAL(...) \
  ::kst::core::Logger::logCore(::kst::core::LogLevel::CRITICAL, KST_LOCATION, __VA_ARGS__)

// Client log macros
#define KST_TRACE(...) \
  ::kst::core::Logger::logApp(::kst::core::LogLevel::TRACE, KST_LOCATION, __VA_ARGS__)
#define KST_DEBUG(...) \
  ::kst::core::Logger::logApp(::kst::core::LogLevel::DEBUG, KST_LOCATION, __VA_ARGS__)
#define KST_INFO(...) \
  ::kst::core::Logger::logApp(::kst::core::LogLevel::INFO, KST_LOCATION, __VA_ARGS__)
#define KST_WARN(...) \
  ::kst::core::Logger::logApp(::kst::core::LogLevel::WARN, KST_LOCATION, __VA_ARGS__)
#define KST_ERROR(...) \
  ::kst::core::Logger::logApp(::kst::core::LogLevel::ERROR, KST_LOCATION, __VA_ARGS__)
#define KST_CRITICAL(...) \
  ::kst::core::Logger::logApp(::kst::core::LogLevel::CRITICAL, KST_LOCATION, __VA_ARGS__)
