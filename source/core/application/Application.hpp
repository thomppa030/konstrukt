#pragma once

#include <memory>

#include "ApplicationWindow.hpp"
#include "Layer.hpp"
#include "LayerStack.hpp"
#include "renderer/Renderer.hpp"

namespace kst::core::application {
  /**
   * @class Application
   * @brief Central engine class that manages the application lifecycle and core systems
   * 
   * The Application class is the entry point and main controller for the engine.
   * It manages:
   * - Window creation and event handling
   * - Renderer initialization and frame execution
   * - Layer stack for modular application components
   * - Main game loop timing and execution
   * 
   * This class follows a singleton-like pattern where a single Application instance
   * is created at program start and manages the entire application lifecycle.
   */
  class Application {
  public:
    Application();
    ~Application();

    // Application is a singleton-like object that should not be copied or moved
    Application(const Application&)                    = delete;
    auto operator=(const Application&) -> Application& = delete;
    Application(Application&&)                         = delete;
    auto operator=(Application&&) -> Application&      = delete;

    /**
     * @brief Initialize the application and all core systems
     * 
     * Creates and configures the application window and renderer based on config settings.
     * Must be called before run().
     */
    void initialize();
    
    /**
     * @brief Run the main application loop until exit is requested
     * 
     * Handles main loop timing, layer updates, rendering, and event processing.
     * Blocks until application exit (window close or escape key).
     */
    void run();
    
    /**
     * @brief Shut down all engine systems in the correct order
     * 
     * Ensures proper cleanup of all resources by detaching layers and
     * shutting down the renderer and window systems.
     */
    void shutdown();

    /**
     * @brief Add a new layer to the application
     * @param layer The layer to add
     * 
     * Layers are added to the middle of the stack, below overlays.
     * They represent the main functional components of the application.
     */
    void pushLayer(std::shared_ptr<Layer> layer);
    
    /**
     * @brief Add a new overlay to the application
     * @param overlay The overlay to add
     * 
     * Overlays are added to the top of the stack, above regular layers.
     * They typically represent UI elements or debug visualization.
     */
    void pushOverlay(std::shared_ptr<Layer> overlay);

    /**
     * @brief Get the application window
     * @return Reference to the window object
     */
    auto getWindow() -> Window& { return *m_window; }

  private:
    // Main window for the application
    std::unique_ptr<Window> m_window;
    
    // Rendering system that manages the graphics pipeline
    std::unique_ptr<kst::renderer::Renderer> m_renderer;
    
    // Stack of application layers and overlays
    std::unique_ptr<LayerStack> m_layerstack;
  };
} // namespace kst::core::application
