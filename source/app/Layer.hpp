#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace kst::app {
  class Context;
  class CommandQueueManager;

  /**
   * @ brief Base Layer class that can be pushed onto the Layer stack
   *
   * The Layer class defines a base interface for all layers in the engine.
   * Layers can represent different parts of the application like rendering,
   * UI, physics, etc. Layers are updated and rendered in the order they are pushed onto the layer
   * stack.
   */
  class Layer {
  public:
    /**
     * @brief Constructor for a Layer
     *
     * @param name The name of the Layer (for debugging)
     */
    explicit Layer(std::string name = "Layer") : m_name(std::move(name)) {};

    virtual ~Layer() = default;

    /**
     * @brief called when the layer is attached to the layer stack
     *
     * Override this to initialize resources needed by the layer
     *
     * @param context The graphics Context
     */
    virtual void onAttach(/**kst::renderer::Context& context*/) = 0;

    /**
     * @brief called when the layer is detached from the layer stack
     *
     * Override this to clean up resources used by the layer
     *
     * @param context The graphics Context
     */
    virtual void onDetach(/**kst::renderer::Context& context*/) = 0;

    /**
     * @brief called to update the layer state
     *
     * Override this to implement the layer's update logic
     *
     * @param deltaTime The time elapsed since the last update
     */
    virtual void onUpdate(float deltaTime) = 0;

    /**
     * @brief called to render the layer
     *
     * Override this to implement the layer's rendering logic
     *
     * @param commandBuffer The command Buffer to record rendering commands into
     * @param swapchainImageIndex The index of the current swapchain image
     */
    virtual void onRender(/**CommandBuffer,*/ uint32_t swapchainImageIndex) = 0;

    /**
     *  @brief called when the window is resized
     *
     *  Override this to handle window resize events
     *
     *  @param width The new width of the window
     *  @param height The new height of the window
     */
    virtual void onResize(uint32_t width, uint32_t height) = 0;

    /**
     *  @brief called when an Event occurs
     *
     *  Override this to handle events. Return true if the event
     *  was handled and should not be propagated to other layers.
     *
     *  @param event The event to handle
     *  @return true if the event was handled, false otherwise
     */
    virtual void onEvent(void* event) = 0;

    auto getName() const -> const std::string& { return m_name; }

    auto isEnabled() const -> bool { return m_enabled; }

  private:
    std::string m_name;
    bool m_enabled{true};
  };
} // namespace kst::app
