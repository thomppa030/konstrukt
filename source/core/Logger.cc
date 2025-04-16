#include "Logger.hpp"

#include <iostream>

#include <spdlog/async.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace kst::core {

  std::shared_ptr<spdlog::logger> Logger::sCoreLogger;
  std::shared_ptr<spdlog::logger> Logger::sClientLogger;
  bool Logger::sInitialized = false;

  void Logger::init(const std::string& logFile, size_t maxFileSize, int maxFiles) {
    if (sInitialized) {
      return;
    }

    try {
      // Set up async logging with a thread pool
      spdlog::init_thread_pool(8192, 1);

      // Set up sinks
      auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
      auto fileSink =
          std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logFile, maxFileSize, maxFiles);

      // Pattern: [time] [logger name] [level] message
      consoleSink->set_pattern("%^[%T] [%n] [%l] %v%$");
      fileSink->set_pattern("[%Y-%m-%d %T.%e] [%n] [%l] %v");

      // Create loggers with both console and file sinks
      std::vector<spdlog::sink_ptr> sinks = {consoleSink, fileSink};

      // Create loggers
      sCoreLogger   = std::make_shared<spdlog::logger>("KONSTRUKT", sinks.begin(), sinks.end());
      sClientLogger = std::make_shared<spdlog::logger>("APP", sinks.begin(), sinks.end());

      // Register loggers with spdlog
      spdlog::register_logger(sCoreLogger);
      spdlog::register_logger(sClientLogger);

      // Set log levels
      sCoreLogger->set_level(spdlog::level::trace);
      sClientLogger->set_level(spdlog::level::trace);

      // Enable flush on error
      sCoreLogger->flush_on(spdlog::level::err);
      sClientLogger->flush_on(spdlog::level::err);

      sInitialized = true;

      // Log initialization
      sCoreLogger->info("Initialized logger");
    } catch (const spdlog::spdlog_ex& ex) {
      std::cerr << "Logger initialization failed: " << ex.what() << std::endl;
    }
  }

  void Logger::shutdown() {
    if (!sInitialized) {
      return;
    }

    sCoreLogger->info("Shutting down logger");
    spdlog::shutdown();
    sInitialized = false;
  }

  void Logger::setLevel(LogLevel level) {
    spdlog::level::level_enum spdlogLevel = toSpdLogLevel(level);
    sCoreLogger->set_level(spdlogLevel);
    sClientLogger->set_level(spdlogLevel);
  }

  auto Logger::getLevel() -> LogLevel {
    auto spdlogLevel = sCoreLogger->level();

    switch (spdlogLevel) {
    case spdlog::level::trace:
      return LogLevel::TRACE;
    case spdlog::level::debug:
      return LogLevel::DEBUG;
    case spdlog::level::info:
      return LogLevel::INFO;
    case spdlog::level::warn:
      return LogLevel::WARN;
    case spdlog::level::err:
      return LogLevel::ERROR;
    case spdlog::level::critical:
      return LogLevel::CRITICAL;
    case spdlog::level::off:
      return LogLevel::OFF;
    default:
      return LogLevel::INFO;
    }
  }

} // namespace kst::core
