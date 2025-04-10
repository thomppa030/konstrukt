#pragma once

#include <string>

#include "renderer/framegraph/FramegraphBuilder.hpp"

namespace kst::core::application {

  /**
   * @class Layer
   * @brief Base class for application layers in the engine's layer system
   * 
   * Layers encapsulate specific application functionality that can be
   * individually enabled/disabled and stacked in a defined order. This pattern allows
   * for modular design where each layer handles a specific aspect of the application
   * like UI, rendering, physics, or game logic.
   * 
   * Layers are ordered from bottom to top, while overlays are always placed on top
   * of regular layers. The rendering and event propagation order is determined by
   * the layer stack order.
   */
  class Layer {
  public:
    /**
     * @brief Constructor for a layer
     * @param name Identifier for the layer, useful for debugging and layer management
     */
    explicit Layer(std::string name = "Layer");
    virtual ~Layer();

    // Layers should not be copied or moved as they represent unique application components
    // with specific lifecycle management through the layer stack
    Layer(const Layer&)                    = delete;
    auto operator=(const Layer&) -> Layer& = delete;
    Layer(Layer&&)                         = delete;
    auto operator=(Layer&&) -> Layer&      = delete;

    /**
     * @brief Called when the layer is added to the layer stack
     * 
     * Intended for initialization that requires the layer to be in the active stack,
     * such as setting up event handlers or allocating resources that depend on
     * the application state.
     */
    virtual void onAttach() {}
    
    /**
     * @brief Called when the layer is removed from the layer stack
     * 
     * Intended to clean up resources allocated in onAttach() or during the layer's lifetime.
     */
    virtual void onDetach() {}

    /**
     * @brief Called each frame to update the layer state
     * @param deltaTime Time elapsed since the last frame in seconds
     * 
     * The main place to implement layer-specific logic that needs to run every frame.
     */
    virtual void onUpdate([[maybe_unused]] float deltaTime) {}

    /**
     * @brief Event handling callback
     * @return True if the event was handled and should not propagate to lower layers
     * 
     * Events propagate from top to bottom in the layer stack. Returning true stops propagation.
     */
    virtual auto onEvent(/*OnEvent*/) -> bool { return false; };
    
    /**
     * @brief Used to register render passes in the framegraph for this layer
     * @param builder The framegraph builder to register render passes with
     * 
     * Called during frame preparation to allow each layer to contribute
     * to the rendering pipeline by registering its render passes in the framegraph.
     */
    virtual void prepareDraw([[maybe_unused]] kst::renderer::framegraph::FrameGraphBuilder& builder) {};

    /**
     * @brief Get the layer's name
     * @return The name of the layer
     */
    auto getName() const -> const std::string& { return m_name; }
    
    /**
     * @brief Check if the layer is currently enabled
     * @return True if the layer is enabled, false otherwise
     */
    auto isEnabled() const -> bool { return m_enabled; }
    
    /**
     * @brief Set the enabled state of the layer
     * @param enabled New enabled state
     * 
     * Disabled layers remain in the layer stack but are skipped during update and rendering.
     * This is more efficient than repeatedly adding/removing layers when they're temporarily unused.
     */
    void setEnabled(bool enabled) { m_enabled = enabled; }

  private:
    std::string m_name;
    bool m_enabled = true;
  };

} // namespace kst::core::application
