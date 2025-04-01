#pragma once

#include <memory>
#include <vector>

#include "Layer.hpp"

namespace kst::core::application {
  class LayerStack {
  public:
    LayerStack() = default;
    ~LayerStack();

    LayerStack(const LayerStack&)                    = delete;
    auto operator=(const LayerStack&) -> LayerStack& = delete;
    LayerStack(LayerStack&&)                         = delete;
    auto operator=(LayerStack&&) -> LayerStack&      = delete;

    void pushLayer(std::shared_ptr<Layer> layer);
    void pushOverlay(std::shared_ptr<Layer> overlay);
    void popLayer(std::shared_ptr<Layer> layer);
    void popOverlay(std::shared_ptr<Layer> overlay);

    auto begin() { return m_layers.begin(); };
    auto end() { return m_layers.end(); };
    auto rbegin() { return m_layers.rbegin(); };
    auto rend() { return m_layers.rend(); };

    auto cbegin() const { return m_layers.begin(); };
    auto cend() const { return m_layers.end(); };
    auto crbegin() const { return m_layers.rbegin(); };
    auto crend() const { return m_layers.rend(); };

  private:
    std::vector<std::shared_ptr<Layer>> m_layers;
    unsigned int m_layerInsertIndex = 0;
  };
} // namespace kst::core::application
