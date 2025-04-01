#include "Framegraph.hpp"

#include <cstddef>
#include <string>
#include <utility>

#include <boost/range/algorithm/find.hpp>

#include "renderer/framegraph/RenderPass.hpp"
#include "renderer/resources/RenderResource.hpp"

namespace kst::renderer::framegraph {
  void FrameGraph::addPass(const RenderPass& pass) {
    m_passes.push_back(pass);
  }

  auto FrameGraph::createResource(std::string& name, const resources::ResourceDesc& desc)
      -> ::kst::core::RenderResourceHandle {
    m_resources[name] = resources::RenderResource(desc);
    m_resources[name].setName(name);
    return name;
  }

  void FrameGraph::addResource(const std::string& name, const resources::RenderResource& resource) {
    m_resources[name] = resource;
  }

  void FrameGraph::compile() {
    // Here we analyze the framegraph to:
    // 1. Validate that all resources are properly produced before consumed
    // 2. Detect and eliminate unnecessary passes (culling)
    // 3. Determine optimal resource allocation and deallocation points
    // 4. Identify opportunities for resource aliasing (reusing memory)

    for (auto& [name, resource] : m_resources) {
      resource.resetUsage();
    }

    std::vector<bool> passUsed(m_passes.size(), false);

    for (size_t i = 0; i < m_passes.size(); ++i) {
      const auto& pass = m_passes[i];
      for (const auto& outputName : pass.getOutputs()) {
        auto* resource = getResource(outputName);

        if ((resource != nullptr) && !resource->isTransient()) {
          passUsed[i] = true;
          resource->markUsed();
          break;
        }
      }
    }

    bool changed = true;
    while (changed) {
      changed = false;

      for (int i = static_cast<int>(m_passes.size()) - 1; i >= 0; --i) {
        if (passUsed[i]) {
          continue;
        }

        const auto& pass = m_passes[i];

        for (const auto& inputName : pass.getInputs()) {
          auto* resource = getResource(inputName);

          if ((resource != nullptr) && !resource->isUsedThisFrame()) {
            resource->markUsed();

            for (size_t j = 0; j < m_passes.size(); ++j) {
              const auto& producerPass = m_passes[j];
              if (boost::find(producerPass.getOutputs(), inputName) !=
                  producerPass.getOutputs().end()) {
                if (!passUsed[j]) {
                  passUsed[j] = true;
                  changed     = true;
                }
                break;
              }
            }
          }
        }
      }
    }

    std::vector<RenderPass> usedPasses;
    for (size_t i = 0; i < m_passes.size(); ++i) {
      if (passUsed[i]) {
        usedPasses.push_back(m_passes[i]);
      }
    }

    m_passes = std::move(usedPasses);
  }

  void FrameGraph::execute(core::GraphicsContext& backend) {
    command::RenderCommandBuffer commandBuffer;

    for (auto& pass : m_passes) {
      for (const auto& input : pass.getInputs()) {
        auto* resource = getResource(input);
        if ((resource != nullptr) && resource->getResourceID().isValid()) {
          if (resource->getState() != ::kst::core::ResourceState::SHADER_READ) {
            backend.transitionResource(
                resource->getResourceID(),
                resource->getState(),
                ::kst::core::ResourceState::SHADER_READ
            );
          }
        }
      }

      for (const auto& output : pass.getOutputs()) {
        auto* resource = getResource(output);
        if ((resource != nullptr) && resource->getResourceID().isValid()) {
          backend.transitionResource(
              resource->getResourceID(),
              resource->getState(),
              ::kst::core::ResourceState::SHADER_WRITE
          );
        }
      }

      commandBuffer.clear();

      pass.execute(commandBuffer);

      if (commandBuffer.getCommandCount() > 0) {
        backend.executeCommands(commandBuffer.getCommands(), commandBuffer.getCommandCount());
      }
    }
  }

  auto FrameGraph::getResource(const std::string& name) -> resources::RenderResource* {
    auto iter = m_resources.find(name);
    if (iter != m_resources.end()) {
      return &iter->second;
    }
    return nullptr;
  }
} // namespace kst::renderer::framegraph
