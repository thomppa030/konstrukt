#pragma once

#include <memory>
#include <string>

#include "renderer/core/GraphicsContext.hpp"
#include "renderer/framegraph/FramegraphBuilder.hpp"

namespace kst::renderer {
  class Renderer {
  public:
    Renderer()  = default;
    ~Renderer() = default;

    void initialize(void* windowHandle, int width, int height);
    void shutdown();

    void beginframe();
    void endFrame();
    void resize(int width, int height);

    auto createFrameGraphBuilder() -> framegraph::FrameGraphBuilder;
    void executeFramegraph(framegraph::FrameGraph& framegraph);

    auto getResourceManager() -> resources::ResourceManager&;

  private:
    std::unique_ptr<core::GraphicsContext> m_context;
    std::unique_ptr<resources::ResourceRegistry> m_registry;
    std::unique_ptr<resources::ResourceManager> m_resourceManager;
    resource::ResourceID m_swapchainResourceID;
    // std::unique_ptr<commands::CommandTranslator> m_commandTranslator;
    framegraph::FrameGraph m_currentFramegraph;

    resource::ResourceID m_swapchainID;
    std::string m_swapchainResourceName{"Swapchain"};
  };
} // namespace kst::renderer
