#include "VulkanTestLayer.hpp"

#include <cmath>

#include <glm/glm.hpp>

#include "core/log/Logger.hpp"
#include "renderer/commands/RenderCommand.hpp"
#include "renderer/commands/RenderCommandBuffer.hpp"

namespace kst::core::application {}

// Clear data structure used for the rendering pass
// Define this in the global namespace so it can be used with templated functions
struct ClearData {
  glm::vec4 clearColor;
};

namespace kst::core::application {

  VulkanTestLayer::VulkanTestLayer() : Layer("VulkanTestLayer") {
    kst::core::Logger::info("Creating VulkanTestLayer");
  }

  void VulkanTestLayer::onAttach() {
    kst::core::Logger::info("VulkanTestLayer attached");
    m_initialized = true;
  }

  void VulkanTestLayer::onDetach() {
    kst::core::Logger::info("VulkanTestLayer detached");
    m_initialized = false;
  }

  void VulkanTestLayer::onUpdate(float deltaTime) {
    if (!m_initialized) {
      return;
    }

    // Animate clear color over time
    m_elapsedTime += deltaTime;
    m_clearColor[0] = (std::sin(m_elapsedTime) + 1.0f) * 0.5f;        // Red
    m_clearColor[1] = (std::sin(m_elapsedTime * 0.5f) + 1.0f) * 0.5f; // Green
    m_clearColor[2] = (std::sin(m_elapsedTime * 0.3f) + 1.0f) * 0.5f; // Blue

    kst::core::Logger::debug<float>("VulkanTestLayer updated, delta={}", deltaTime);
  }

  void VulkanTestLayer::prepareDraw(
      kst::renderer::framegraph::FrameGraphBuilder& builder [[maybe_unused]]
  ) {
    if (!m_initialized) {
      return;
    }

    // Very simple clear pass to test the Vulkan context
    builder.addPass<ClearData, ClearData>(
        "ClearScreen",
        // Setup function
        [this](kst::renderer::framegraph::PassBuilder& passBuilder) -> ClearData {
          // Import swapchain as color target
          passBuilder.write("Swapchain");

          ClearData data{};
          data.clearColor =
              glm::vec4(m_clearColor[0], m_clearColor[1], m_clearColor[2], m_clearColor[3]);
          return data;
        },
        // Execute function
        [](const ClearData& data, kst::renderer::command::RenderCommandBuffer& cmdBuffer) {
          // In a real implementation, this would use actual command buffer to clear the screen
          kst::core::Logger::debug<float, float, float, float>(
              "Executing clear screen pass with color ({}, {}, {}, {})",
              data.clearColor.r,
              data.clearColor.g,
              data.clearColor.b,
              data.clearColor.a
          );

          // Create clear command
          kst::renderer::command::ClearCommandData clearCmd{};
          clearCmd.color   = data.clearColor;
          clearCmd.depth   = 1.0F;
          clearCmd.stencil = 0;
          clearCmd.flags   = kst::renderer::command::ClearFlags::COLOR;

          // Submit clear command to the command buffer
          cmdBuffer.submit(kst::renderer::command::RenderCommandType::CLEAR, clearCmd);
        }
    );
  }

} // namespace kst::core::application
