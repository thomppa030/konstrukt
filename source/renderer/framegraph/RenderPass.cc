#include "RenderPass.hpp"

#include <boost/range/algorithm/find.hpp>

namespace kst::renderer::framegraph {

  auto RenderPass::getName() const -> const std::string& {
    return m_name;
  }

  void RenderPass::setName(const std::string& name) {
    m_name = name;
  }

  void RenderPass::addInput(const core::RenderResourceHandle& resource) {
    if (boost::find(m_inputs, resource) == m_inputs.end()) {
      m_inputs.push_back(resource);
    }
  }

  void RenderPass::addOutput(const core::RenderResourceHandle& resource) {
    if (boost::find(m_outputs, resource) == m_outputs.end()) {
      m_outputs.push_back(resource);
    }
  }

  void RenderPass::setExecuteFunction(std::function<void(command::RenderCommandBuffer&)> func) {
    m_executeFunc = std::move(func);
  };

  auto RenderPass::getOutputs() const -> const std::vector<core::RenderResourceHandle>& {
    return m_outputs;
  }

  auto RenderPass::getInputs() const -> const std::vector<core::RenderResourceHandle>& {
    return m_inputs;
  }

  void RenderPass::execute(command::RenderCommandBuffer& commands) {
    if (m_executeFunc) {
      m_executeFunc(commands);
    }
  }

} // namespace kst::renderer::framegraph
