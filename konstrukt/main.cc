#include <filesystem>
#include <memory>

#include "core/application/Application.hpp"
#include "core/application/VulkanTestLayer.hpp"
#include "core/config/Config.hpp"
#include "core/log/Logger.hpp"

auto main(int /*argc*/, char* /*argv*/[]) -> int {
  // Initialize the logger
  kst::core::Logger::init();
  kst::core::Logger::info<>("Starting Konstrukt engine...");

  // Load configuration with file watching enabled
  if (kst::core::Config::init("config.json", true)) {
    kst::core::Logger::info<>("Loaded configuration from config.json with file watching enabled");

    // Function to set log level from config
    auto updateLogLevel = [](const std::string& level) {
      if (level == "trace") {
        kst::core::Logger::setLevel(kst::core::LogLevel::TRACE);
      } else if (level == "debug") {
        kst::core::Logger::setLevel(kst::core::LogLevel::DEBUG);
      } else if (level == "info") {
        kst::core::Logger::setLevel(kst::core::LogLevel::INFO);
      } else if (level == "warn") {
        kst::core::Logger::setLevel(kst::core::LogLevel::WARN);
      } else if (level == "error") {
        kst::core::Logger::setLevel(kst::core::LogLevel::ERROR);
      } else if (level == "critical") {
        kst::core::Logger::setLevel(kst::core::LogLevel::CRITICAL);
      } else {
        kst::core::Logger::setLevel(kst::core::LogLevel::INFO);
      }
    };

    // Set initial log level
    std::string logLevel = kst::core::Config::getString("logging.level", "info");
    updateLogLevel(logLevel);

    // Register callback for log level changes
    kst::core::Config::onValueChanged(
        "logging.level",
        [updateLogLevel](const std::string& /*key*/, const nlohmann::json& value) {
          kst::core::Logger::info<const std::string&>(
              "Log level changed to: {}",
              value.get<std::string>()
          );
          updateLogLevel(value.get<std::string>());
        }
    );

    // Register callbacks for window settings
    kst::core::Config::onValueChanged(
        "window.width",
        [](const std::string& /*key*/, const nlohmann::json& value) {
          kst::core::Logger::info<int>("Window width config changed to: {}", value.get<int>());
        }
    );

    kst::core::Config::onValueChanged(
        "window.height",
        [](const std::string& /*key*/, const nlohmann::json& value) {
          kst::core::Logger::info<int>("Window height config changed to: {}", value.get<int>());
        }
    );

    // Register a global callback for debug purposes
    kst::core::Config::onAnyValueChanged([](const std::string& key, const nlohmann::json& value) {
      // Convert the value to a string for logging, regardless of type
      std::string valueStr;
      if (value.is_string()) {
        valueStr = value.get<std::string>();
      } else if (value.is_number_integer()) {
        valueStr = std::to_string(value.get<int>());
      } else if (value.is_number_float()) {
        valueStr = std::to_string(value.get<float>());
      } else if (value.is_boolean()) {
        valueStr = value.get<bool>() ? "true" : "false";
      } else {
        valueStr = "<complex value>";
      }

      kst::core::Logger::debug<const std::string&, const std::string&>(
          "Config changed: {} = {}",
          key,
          valueStr
      );
    });
  } else {
    kst::core::Logger::warn<>("Failed to load config.json, using default settings");
    kst::core::Logger::setLevel(kst::core::LogLevel::INFO);
  }

  try {
    // Create and initialize the application
    kst::core::application::Application app;
    app.initialize();

    // Create and add our Vulkan test layer
    auto vulkanTestLayer = std::make_shared<kst::core::application::VulkanTestLayer>();
    app.pushLayer(vulkanTestLayer);

    // Run the application main loop
    kst::core::Logger::info<>("Running main application loop");
    app.run();

    // Shutdown the application
    app.shutdown();
  } catch (const std::exception& e) {
    kst::core::Logger::critical<const char*>("Fatal error: {}", e.what());
    return 1;
  }

  kst::core::Logger::info<>("Konstrukt engine shutting down normally");
  kst::core::Logger::shutdown();

  return 0;
}
