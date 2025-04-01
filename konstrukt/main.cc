#include <memory>

#include "core/application/Application.hpp"
#include "core/application/VulkanTestLayer.hpp"
#include "core/log/Logger.hpp"

auto main(int /*argc*/, char* /*argv*/[]) -> int {
  // Initialize the logger
  kst::core::Logger::init();
  kst::core::Logger::info<>("Starting Konstrukt engine...");
  kst::core::Logger::setLevel(kst::core::LogLevel::INFO);

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
