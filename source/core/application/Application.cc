#include "Application.hpp"

#include <memory>
#include <string>
#include <utility>

#include <GLFW/glfw3.h>

#include "ApplicationWindow.hpp"
#include "Layer.hpp"
#include "LayerStack.hpp"
#include "core/log/Logger.hpp"
#include "renderer/Renderer.hpp"

namespace kst::core::application {

  Application::Application() {
    kst::core::Logger::info("Application constructor");
    m_layerstack = std::make_unique<LayerStack>();
    m_window     = std::make_unique<Window>();
  }

  Application::~Application() {
    kst::core::Logger::info("Application destructor");
    m_layerstack.reset();
    m_window.reset();
    m_renderer.reset();
  }

  void Application::initialize() {
    kst::core::Logger::info("Initializing application");

    // Create the window
    const std::string title = "Konstrukt Engine";
    const int width         = 1280;
    const int height        = 720;

    if (!m_window->create(title, width, height)) {
      throw std::runtime_error("Failed to create application window");
    }

    // Create and initialize the renderer
    m_renderer = std::make_unique<kst::renderer::Renderer>();
    m_renderer->initialize(m_window->getNativeWindow(), width, height);

    // Set the resize callback
    m_window->setResizeCallback([this](int width, int height) {
      kst::core::Logger::info<int, int>("Window resized to {}x{}", width, height);
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

    while (running && !m_window->shouldClose()) {
      // Poll for window events
      m_window->pollEvents();

      // Calculate delta time
      auto time             = static_cast<float>(glfwGetTime());
      const float deltaTime = time - lastFrameTime;
      lastFrameTime         = time;

      // Check for escape key to exit
      auto* window = static_cast<GLFWwindow*>(m_window->getNativeWindow());
      if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        running = false;
        glfwSetWindowShouldClose(window, GLFW_TRUE);
      }

      // Begin frame and render
      m_renderer->beginframe();
      auto framegraphBuilder = m_renderer->createFrameGraphBuilder();

      // Update and prepare drawing for all active layers
      for (auto& layer : *m_layerstack) {
        if (layer->isEnabled()) {
          layer->onUpdate(deltaTime);
          layer->prepareDraw(framegraphBuilder);
        }
      }

      // Build and execute the framegraph
      kst::renderer::framegraph::FrameGraph frameGraph = framegraphBuilder.build();
      m_renderer->executeFramegraph(frameGraph);

      // End frame and present
      m_renderer->endFrame();
    }

    kst::core::Logger::info("Application main loop ended");
  }

  void Application::shutdown() {
    kst::core::Logger::info("Shutting down application");

    // Detach all layers
    for (auto& layer : *m_layerstack) {
      layer->onDetach();
    }

    // Shutdown renderer and window
    if (m_renderer) {
      m_renderer->shutdown();
    }

    if (m_window) {
      m_window->destroy();
    }

    kst::core::Logger::info("Application shutdown complete");
  }
} // namespace kst::core::application
