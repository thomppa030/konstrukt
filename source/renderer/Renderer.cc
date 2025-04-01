#include "Renderer.hpp"

#include <cstdint>
#include <memory>
#include <string>

#include "GraphicsType.hpp"
#include "core/log/Logger.hpp"
#include "renderer/core/GraphicsContext.hpp"
#include "renderer/core/vulkan/VulkanContext.hpp"
#include "renderer/framegraph/FramegraphBuilder.hpp"
#include "renderer/resources/ResourceManager.hpp"
#include "renderer/resources/ResourceRegistry.hpp"
#include "resources/RenderResource.hpp"

namespace kst::renderer {

  void Renderer::initialize(void* windowHandle, int width, int height) {
    kst::core::Logger::info("Initializing renderer");

    // Create the graphics context
    m_context = std::make_unique<core::VulkanContext>();

    // Initialize the graphics context
    if (!m_context->initialize(
            windowHandle,
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        )) {
      throw std::runtime_error("Failed to initialize graphics context");
    }

    // Create resource registry and manager
    m_registry        = std::make_unique<resources::ResourceRegistry>();
    m_resourceManager = std::make_unique<resources::ResourceManager>(*m_context, *m_registry);

    m_context->registerSwapchainResource(m_swapchainID);

    m_registry->registerTexture(m_swapchainID, {});

    kst::core::Logger::info("Renderer initialization completed");
  }

  void Renderer::shutdown() {
    kst::core::Logger::info("Shutting down renderer");

    // First reset the resource managers
    if (m_resourceManager) {
      m_resourceManager.reset();
      kst::core::Logger::debug("Resource manager destroyed");
    }

    if (m_registry) {
      m_registry.reset();
      kst::core::Logger::debug("Resource registry destroyed");
    }

    // Then shutdown graphics context (which will handle safe destruction of all Vulkan objects)
    if (m_context) {
      // Wait for all GPU operations to complete before destruction
      try {
        m_context->waitForIdle();
      } catch (const std::exception& e) {
        kst::core::Logger::error<const char*>("Error waiting for device idle: {}", e.what());
      }

      try {
        // Reset the context which will trigger its destructor
        m_context.reset();
        kst::core::Logger::debug("Graphics context destroyed");
      } catch (const std::exception& e) {
        kst::core::Logger::error<const char*>("Error destroying context: {}", e.what());
      }
    }

    kst::core::Logger::info("Renderer shutdown completed");
  }

  void Renderer::beginframe() {
    // Acquire the next swapchain image
    uint32_t imageIndex = m_context->beginFrame();
    kst::core::Logger::debug<uint32_t>("Begin frame, image index: {}", imageIndex);

    m_currentFramegraph = framegraph::FrameGraph();

    uint32_t width  = 0;
    uint32_t height = 0;

    resources::TextureDesc swapchainDesc;
    swapchainDesc.width       = width;
    swapchainDesc.height      = height;
    swapchainDesc.depth       = 1;
    swapchainDesc.mipLevels   = 1;
    swapchainDesc.arrayLevels = 1;
    swapchainDesc.cubeMap     = false;
    swapchainDesc.format      = m_context->getSwapchainFormat();

    resources::ResourceDesc swapchainResourceDesc(
        ::kst::core::ResourceType::TEXTURE,
        swapchainDesc
    );

    m_currentFramegraph.createResource(m_swapchainResourceName, swapchainResourceDesc);

    auto* swapchainResource = m_currentFramegraph.getResource(m_swapchainResourceName);

    if (swapchainResource != nullptr) {
      swapchainResource->setResourceID(m_swapchainID);
      swapchainResource->setState(::kst::core::ResourceState::RENDER_TARGET);
      swapchainResource->setTransient(/*transient=*/false);
    }
  }

  void Renderer::endFrame() {
    // Present the rendered image
    m_context->endFrame();
    kst::core::Logger::debug("End frame");
  }

  void Renderer::resize(int width, int height) {
    kst::core::Logger::info<int, int>("Resizing renderer to {}x{}", width, height);

    // Wait for any ongoing operations to complete
    m_context->waitForIdle();

    // Resize the graphics context's swapchain
    m_context->resize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
  }

  auto Renderer::createFrameGraphBuilder() -> framegraph::FrameGraphBuilder {
    return framegraph::FrameGraphBuilder(m_currentFramegraph, *m_registry, *m_resourceManager);
  }

  void Renderer::executeFramegraph(framegraph::FrameGraph& framegraph [[maybe_unused]]) {
    framegraph.compile();
    framegraph.execute(*m_context);
  }

  auto Renderer::getResourceManager() -> resources::ResourceManager& {
    return *m_resourceManager;
  }

} // namespace kst::renderer
