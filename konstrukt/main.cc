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

  // Load configuration
  if (kst::core::Config::init("config.json")) {
    kst::core::Logger::info<>("Loaded configuration from config.json");

    // Set log level from config
    std::string logLevel = kst::core::Config::getString("logging.level", "info");
    if (logLevel == "trace") {
      kst::core::Logger::setLevel(kst::core::LogLevel::TRACE);
    } else if (logLevel == "debug") {
      kst::core::Logger::setLevel(kst::core::LogLevel::DEBUG);
    } else if (logLevel == "info") {
      kst::core::Logger::setLevel(kst::core::LogLevel::INFO);
    } else if (logLevel == "warn") {
      kst::core::Logger::setLevel(kst::core::LogLevel::WARN);
    } else if (logLevel == "error") {
      kst::core::Logger::setLevel(kst::core::LogLevel::ERROR);
    } else if (logLevel == "critical") {
      kst::core::Logger::setLevel(kst::core::LogLevel::CRITICAL);
    } else {
      kst::core::Logger::setLevel(kst::core::LogLevel::INFO);
    }
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
