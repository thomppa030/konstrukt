#include "Application.hpp"

#include <memory>
#include <string>
#include <utility>

#include <GLFW/glfw3.h>

#include "ApplicationWindow.hpp"
#include "Layer.hpp"
#include "LayerStack.hpp"
#include "core/config/Config.hpp"
#include "core/log/Logger.hpp"
#include "renderer/Renderer.hpp"

namespace kst::core::application {

  Application::Application() {
    kst::core::Logger::info("Application constructor");
    // Create core systems in a specific order to manage dependencies correctly
    // LayerStack has no dependencies, so create it first
    m_layerstack = std::make_unique<LayerStack>();
    // Window is created next but not initialized until initialize() is called
    m_window     = std::make_unique<Window>();
    // Renderer is created last in initialize() as it depends on the window
  }

  Application::~Application() {
    kst::core::Logger::info("Application destructor");
    // Destroy systems in reverse order of creation to handle dependencies
    m_layerstack.reset();  // Destroy layers first (may reference window/renderer)
    m_window.reset();      // Destroy window after layers
    m_renderer.reset();    // Destroy renderer last (depends on window)
  }

  void Application::initialize() {
    kst::core::Logger::info("Initializing application");

    // Load window settings from config with sensible defaults if not found
    // This allows for easy configuration without code changes
    const std::string title = kst::core::Config::getString("window.title", "Konstrukt Engine");
    const int width         = kst::core::Config::getInt("window.width", 1280);
    const int height        = kst::core::Config::getInt("window.height", 720);
    const bool fullscreen   = kst::core::Config::getBool("window.fullscreen", false);
    const bool resizable    = kst::core::Config::getBool("window.resizable", true);

    kst::core::Logger::info<int, int, bool, bool>(
        "Creating window: {}x{}, fullscreen: {}, resizable: {}",
        width,
        height,
        fullscreen,
        resizable
    );

    // Create the window with the configured settings
    if (!m_window->create(title, width, height, fullscreen, resizable)) {
      throw std::runtime_error("Failed to create application window");
    }

    // Create and initialize the renderer after window is created
    // Renderer needs a valid window handle to initialize properly
    m_renderer = std::make_unique<kst::renderer::Renderer>();

    // Load renderer settings from config with sensible defaults
    const std::string api       = kst::core::Config::getString("renderer.api", "vulkan");
    const int msaa              = kst::core::Config::getInt("renderer.msaa", 1);
    const int maxFramesInFlight = kst::core::Config::getInt("renderer.maxFramesInFlight", 2);

    kst::core::Logger::info<std::string, int, int>(
        "Initializing renderer: API={}, MSAA={}x, MaxFramesInFlight={}",
        api,
        msaa,
        maxFramesInFlight
    );

    // Initialize renderer with window information
    m_renderer->initialize(m_window->getNativeWindow(), width, height);

    // Set window resize handler to update renderer
    // Using a lambda to maintain the this pointer context
    m_window->setResizeCallback([this](int width, int height) {
      kst::core::Logger::info<int, int>("Window resized to {}x{}", width, height);
      // Renderer needs to recreate swapchain and framebuffers on resize
      m_renderer->resize(width, height);
    });

    kst::core::Logger::info("Application initialized successfully");
  }

  void Application::pushLayer(std::shared_ptr<Layer> layer) {
    kst::core::Logger::info<const std::string&>("Pushing layer: {}", layer->getName());
    m_layerstack->pushLayer(std::move(layer));
  }

  void Application::pushOverlay(std::shared_ptr<Layer> overlay) {
    kst::core::Logger::info<const std::string&>("Pushing overlay: {}", overlay->getName());
    m_layerstack->pushOverlay(std::move(overlay));
  }

  void Application::run() {
    kst::core::Logger::info("Starting application main loop");

    bool running        = true;
    float lastFrameTime = static_cast<float>(glfwGetTime());

    // Main application loop
    while (running && !m_window->shouldClose()) {
      // Process window events (mouse, keyboard, window management)
      m_window->pollEvents();

      // Calculate frame delta time for consistent animation and physics
      auto time             = static_cast<float>(glfwGetTime());
      const float deltaTime = time - lastFrameTime;
      lastFrameTime         = time;

      // Quick exit with escape key for development convenience
      // In a production application, this would be handled through proper input system
      auto* window = static_cast<GLFWwindow*>(m_window->getNativeWindow());
      if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        running = false;
        glfwSetWindowShouldClose(window, GLFW_TRUE);
      }

      // Start renderer frame
      m_renderer->beginframe();
      
      // Create framegraph builder for this frame
      auto framegraphBuilder = m_renderer->createFrameGraphBuilder();

      // Update and prepare drawing for all active layers
      // Layers aren't updated if disabled, saving processing time
      for (auto& layer : *m_layerstack) {
        if (layer->isEnabled()) {
          // Update layer logic
          layer->onUpdate(deltaTime);
          // Let layer register render passes in the framegraph
          layer->prepareDraw(framegraphBuilder);
        }
      }

      // Build the framegraph for this frame
      // This analyzes dependencies and optimizes the rendering order
      kst::renderer::framegraph::FrameGraph frameGraph = framegraphBuilder.build();
      
      // Execute the compiled framegraph
      // This translates high-level render passes into actual graphics commands
      m_renderer->executeFramegraph(frameGraph);

      // End frame and present to screen
      m_renderer->endFrame();
    }

    kst::core::Logger::info("Application main loop ended");
  }

  void Application::shutdown() {
    kst::core::Logger::info("Shutting down application");

    // Detach all layers first to let them clean up resources
    // This is important as layers might hold references to renderer resources
    for (auto& layer : *m_layerstack) {
      layer->onDetach();
    }

    // Shutdown renderer before window
    // Renderer depends on window, so it must be shut down first
    if (m_renderer) {
      m_renderer->shutdown();
    }

    // Destroy window last
    if (m_window) {
      m_window->destroy();
    }

    kst::core::Logger::info("Application shutdown complete");
  }
} // namespace kst::core::application
