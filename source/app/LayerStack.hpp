#pragma once

#include <memory>

#include "Layer.hpp"

namespace kst::app {
  class LayerStack {
  public:
    LayerStack() = default;
    ~LayerStack();

    // Copying or moving a LayerStack would duplicate layer references and cause
    // ambiguity in layer ownership and lifecycle management
    LayerStack(const LayerStack&)                    = delete;
    auto operator=(const LayerStack&) -> LayerStack& = delete;
    LayerStack(LayerStack&&)                         = delete;
    auto operator=(LayerStack&&) -> LayerStack&      = delete;

    /**
     * @brief Add a layer to the stack (inserted below overlays)
     * @param layer Shared pointer to the layer being added
     *
     * Regular layers are inserted at m_layerInsertIndex, which separates
     * regular layers from overlays in the internal vector.
     */
    void pushLayer(std::shared_ptr<Layer> layer);

    /**
     * @brief Add an overlay to the stack (always at the top)
     * @param overlay Shared pointer to the overlay being added
     *
     * Overlays are always added at the end of the vector, ensuring they
     * are processed after (rendered on top of) regular layers.
     */
    void pushOverlay(std::shared_ptr<Layer> overlay);

    /**
     * @brief Remove a regular layer from the stack
     * @param layer Shared pointer to the layer being removed
     *
     * Searches for the layer in the regular layer region and removes it if found.
     */
    void popLayer(std::shared_ptr<Layer> layer);

    /**
     * @brief Remove an overlay from the stack
     * @param overlay Shared pointer to the overlay being removed
     *
     * Searches for the overlay in the overlay region and removes it if found.
     */
    void popOverlay(std::shared_ptr<Layer> overlay);

    // Iterator methods to allow range-based for loops and standard algorithms.
    // Forward iterators go from bottom to top layer (regular layers first, then overlays)
    auto begin() { return m_layers.begin(); };
    auto end() { return m_layers.end(); };

    // Reverse iterators go from top to bottom (overlays first, then regular layers)
    // This is useful for event propagation which goes from top to bottom
    auto rbegin() { return m_layers.rbegin(); };
    auto rend() { return m_layers.rend(); };

    // Const iterators for const LayerStack instances
    auto cbegin() const { return m_layers.begin(); };
    auto cend() const { return m_layers.end(); };
    auto crbegin() const { return m_layers.rbegin(); };
    auto crend() const { return m_layers.rend(); };

  private:
    // TODO shared_ptr to Graphics Context after implementation of such

    // Single vector containing both regular layers and overlays
    std::vector<std::shared_ptr<Layer>> m_layers;

    // Index that separates regular layers from overlays
    // Regular layers: [0, m_layerInsertIndex)
    // Overlays: [m_layerInsertIndex, end)
    unsigned int m_layerInsertIndex = 0;
  };
}; // namespace kst::app
