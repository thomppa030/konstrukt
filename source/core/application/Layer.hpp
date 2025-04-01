#pragma once

#include <string>

#include "renderer/framegraph/FramegraphBuilder.hpp"

namespace kst::core::application {

  class Layer {
  public:
    explicit Layer(std::string name = "Layer");
    virtual ~Layer();

    Layer(const Layer&)                    = delete;
    auto operator=(const Layer&) -> Layer& = delete;
    Layer(Layer&&)                         = delete;
    auto operator=(Layer&&) -> Layer&      = delete;

    virtual void onAttach() {}
    virtual void onDetach() {}

    virtual void onUpdate([[maybe_unused]] float deltaTime) {}

    virtual auto onEvent(/*OnEvent*/) -> bool { return false; };
    virtual void prepareDraw([[maybe_unused]] kst::renderer::framegraph::FrameGraphBuilder& builder) {};

    auto getName() const -> const std::string& { return m_name; }
    auto isEnabled() const -> bool { return m_enabled; }
    void setEnabled(bool enabled) { m_enabled = enabled; }

  private:
    std::string m_name;
    bool m_enabled = true;
  };

} // namespace kst::core::application
