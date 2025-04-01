#pragma once

#include <functional>
#include <string>
#include <vector>

#include "core/CoreTypes.hpp"
#include "renderer/commands/RenderCommandBuffer.hpp"

namespace kst::renderer::framegraph {
  class RenderPass {
  public:
    auto getName() const -> const std::string&;
    void setName(const std::string& name);
    void addInput(const ::kst::core::RenderResourceHandle& resource);
    void addOutput(const ::kst::core::RenderResourceHandle& resource);

    auto getOutputs() const -> const std::vector<::kst::core::RenderResourceHandle>&;
    auto getInputs() const -> const std::vector<::kst::core::RenderResourceHandle>&;

    void setExecuteFunction(std::function<void(command::RenderCommandBuffer&)> func);

    void execute(command::RenderCommandBuffer& commands);

  private:
    std::string m_name;
    std::vector<::kst::core::RenderResourceHandle> m_inputs;
    std::vector<::kst::core::RenderResourceHandle> m_outputs;
    std::function<void(command::RenderCommandBuffer&)> m_executeFunc;
  };
} // namespace kst::renderer::framegraph
