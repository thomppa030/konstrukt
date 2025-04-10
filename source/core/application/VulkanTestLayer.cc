#include "VulkanTestLayer.hpp"

#include <cmath>

#include <glm/glm.hpp>

#include "core/log/Logger.hpp"
#include "renderer/commands/RenderCommand.hpp"
#include "renderer/commands/RenderCommandBuffer.hpp"

namespace kst::core::application {}

/**
 * @struct ClearData
 * @brief Data structure for the clear screen render pass
 * 
 * Defined in global namespace to work with framegraph template functions.
 * The framegraph system uses template parameters for pass data, and C++
 * template argument deduction works better with types in the global namespace.
 */
struct ClearData {
  glm::vec4 clearColor;
};

namespace kst::core::application {

  VulkanTestLayer::VulkanTestLayer() : Layer("VulkanTestLayer") {
    kst::core::Logger::info("Creating VulkanTestLayer");
  }

  void VulkanTestLayer::onAttach() {
    // Log attachment for debugging
    kst::core::Logger::info("VulkanTestLayer attached");
    // Enable rendering only after the layer is properly initialized
    m_initialized = true;
  }

  void VulkanTestLayer::onDetach() {
    // Log detachment for debugging
    kst::core::Logger::info("VulkanTestLayer detached");
    // Disable rendering to prevent accessing invalid resources
    m_initialized = false;
  }

  void VulkanTestLayer::onUpdate(float deltaTime) {
    if (!m_initialized) {
      return;
    }

    // Animate clear color over time using sine waves for smooth transitions
    // Different frequencies for each channel create a more interesting effect
    m_elapsedTime += deltaTime;
    m_clearColor[0] = (std::sin(m_elapsedTime) + 1.0f) * 0.5f;        // Red   (1.0 Hz)
    m_clearColor[1] = (std::sin(m_elapsedTime * 0.5f) + 1.0f) * 0.5f; // Green (0.5 Hz)
    m_clearColor[2] = (std::sin(m_elapsedTime * 0.3f) + 1.0f) * 0.5f; // Blue  (0.3 Hz)

    kst::core::Logger::debug<float>("VulkanTestLayer updated, delta={}", deltaTime);
  }

  void VulkanTestLayer::prepareDraw(
      kst::renderer::framegraph::FrameGraphBuilder& builder [[maybe_unused]]
  ) {
    if (!m_initialized) {
      return;
    }

    // Register a render pass in the framegraph to clear the screen
    // This demonstrates the minimal required implementation of a render pass
    builder.addPass<ClearData, ClearData>(
        "ClearScreen",
        // Setup function - runs during framegraph compilation to set up resource dependencies
        [this](kst::renderer::framegraph::PassBuilder& passBuilder) -> ClearData {
          // Declare that this pass writes to the swapchain (output image)
          // This creates appropriate resource barriers and synchronization
          passBuilder.write("Swapchain");

          // Prepare and return pass data (clear color)
          ClearData data{};
          data.clearColor =
              glm::vec4(m_clearColor[0], m_clearColor[1], m_clearColor[2], m_clearColor[3]);
          return data;
        },
        // Execute function - runs during frame rendering to issue actual GPU commands
        [](const ClearData& data, kst::renderer::command::RenderCommandBuffer& cmdBuffer) {
          // Log the clear operation for debugging
          kst::core::Logger::debug<float, float, float, float>(
              "Executing clear screen pass with color ({}, {}, {}, {})",
              data.clearColor.r,
              data.clearColor.g,
              data.clearColor.b,
              data.clearColor.a
          );

          // Create a clear command with the appropriate data
          kst::renderer::command::ClearCommandData clearCmd{};
          clearCmd.color   = data.clearColor;
          clearCmd.depth   = 1.0F;  // Default depth clear value (far plane)
          clearCmd.stencil = 0;     // Default stencil clear value
          clearCmd.flags   = kst::renderer::command::ClearFlags::COLOR;  // Only clear color buffer

          // Submit the command to the command buffer for execution
          // The actual Vulkan command recording happens in the command translator
          cmdBuffer.submit(kst::renderer::command::RenderCommandType::CLEAR, clearCmd);
        }
    );
  }

} // namespace kst::core::application
