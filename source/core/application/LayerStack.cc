#include "LayerStack.hpp"

#include <algorithm>
#include <memory>
#include <string>

#include "Layer.hpp"
#include "core/log/Logger.hpp"

namespace kst::core::application {
  LayerStack::~LayerStack() {
    for (auto& layer : m_layers) {
      layer->onDetach();
    }
    m_layers.clear();
  }

  void LayerStack::pushLayer(std::shared_ptr<Layer> layer) {
    kst::core::Logger::info<const std::string&>("Adding Layer: ", layer->getName());
    m_layers.emplace(m_layers.begin() + m_layerInsertIndex, layer);
    m_layerInsertIndex++;
    layer->onAttach();
  }

  void LayerStack::pushOverlay(std::shared_ptr<Layer> overlay) {
    kst::core::Logger::info<const std::string&>("Adding Overlay: ", overlay->getName());
    m_layers.emplace_back(overlay);
    overlay->onAttach();
  }

  void LayerStack::popLayer(std::shared_ptr<Layer> layer) {
    auto iter = std::find(m_layers.begin(), m_layers.begin() + m_layerInsertIndex, layer);
    if (iter != m_layers.begin() + m_layerInsertIndex) {
      kst::core::Logger::info<const std::string&>("Removing Layer: {}", layer->getName());
      layer->onDetach();
      m_layers.erase(iter);
      m_layerInsertIndex--;
    }
  }

  void LayerStack::popOverlay(std::shared_ptr<Layer> overlay) {
    auto iter = std::find(m_layers.begin() + m_layerInsertIndex, m_layers.end(), overlay);
    if (iter != m_layers.end()) {
      kst::core::Logger::info<const std::string&>("Removing Overlay: {}", overlay->getName());
      overlay->onDetach();
      m_layers.erase(iter);
    }
  }
} // namespace kst::core::application
