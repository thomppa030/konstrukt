#pragma once

#include <memory>

#include "Layer.hpp"
#include "renderer/core/GraphicsContext.hpp"
#include "renderer/core/vulkan/VulkanContext.hpp"
#include "renderer/framegraph/FramegraphBuilder.hpp"

struct ClearData;

namespace kst::core::application {

  /**
   * @class VulkanTestLayer
   * @brief Test layer for demonstrating Vulkan functionality
   *
   * This layer serves as a minimal working example of the renderer architecture:
   * - Demonstrates the framegraph-based rendering approach
   * - Shows how to create and configure a simple render pass
   * - Provides a visual indication that the rendering pipeline is working
   * - Uses an animated clear color as a simple visual effect that requires no resources
   * 
   * The implementation is intentionally simple to serve as a starting point and
   * verification tool during early engine development, before more complex rendering
   * features are implemented and tested.
   */
  class VulkanTestLayer : public Layer {
  public:
    VulkanTestLayer();
    ~VulkanTestLayer() override = default;

    void onAttach() override;
    void onDetach() override;
    void onUpdate(float deltaTime) override;

    void prepareDraw(kst::renderer::framegraph::FrameGraphBuilder& builder) override;

  private:
    // Initial clear color (dark blue) - will be animated during updates
    float m_clearColor[4] = {0.1f, 0.1f, 0.3f, 1.0f};
    
    // Tracks running time for color animation
    float m_elapsedTime   = 0.0f;
    
    // Flag to prevent rendering before proper initialization
    // This prevents crashes when trying to access resources before they're ready
    bool m_initialized    = false;
  };

} // namespace kst::core::application

