#pragma once

#include <memory>

#include "ApplicationWindow.hpp"
#include "Layer.hpp"
#include "LayerStack.hpp"
#include "renderer/Renderer.hpp"

namespace kst::core::application {
  class Application {
  public:
    Application();
    ~Application();

    Application(const Application&)                    = delete;
    auto operator=(const Application&) -> Application& = delete;
    Application(Application&&)                         = delete;
    auto operator=(Application&&) -> Application&      = delete;

    void initialize();
    void run();
    void shutdown();

    void pushLayer(std::shared_ptr<Layer> layer);
    void pushOverlay(std::shared_ptr<Layer> overlay);

    auto getWindow() -> Window& { return *m_window; }

  private:
    std::unique_ptr<Window> m_window;
    std::unique_ptr<kst::renderer::Renderer> m_renderer;
    std::unique_ptr<LayerStack> m_layerstack;
  };
} // namespace kst::core::application
