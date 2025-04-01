#pragma once

#include <memory>

#include "Layer.hpp"
#include "renderer/core/GraphicsContext.hpp"
#include "renderer/core/vulkan/VulkanContext.hpp"
#include "renderer/framegraph/FramegraphBuilder.hpp"

struct ClearData;

namespace kst::core::application {

  /**
   * Test layer for demonstrating Vulkan functionality
   *
   * This layer tests the basic functionality of the Vulkan renderer
   * by creating a simple render pass that clears the screen.
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
    float m_clearColor[4] = {0.1f, 0.1f, 0.3f, 1.0f};
    float m_elapsedTime   = 0.0f;
    bool m_initialized    = false;
  };

} // namespace kst::core::application

