#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <utility>

#include "Framegraph.hpp"
#include "renderer/resources/ResourceID.hpp"
#include "renderer/resources/ResourceManager.hpp"
#include "renderer/resources/ResourceRegistry.hpp"

namespace kst::renderer::command { class RenderCommandBuffer; }

namespace kst::renderer::framegraph {

  class PassBuilder {
  public:
    void read(const std::string& resourceName);
    void write(const std::string& resourceName);

    auto getInputs() const -> const std::vector<std::string>& { return m_inputs; };
    auto getOutputs() const -> const std::vector<std::string>& { return m_outputs; };

  private:
    std::vector<std::string> m_inputs;
    std::vector<std::string> m_outputs;
  };

  class FrameGraphBuilder {
  public:
    FrameGraphBuilder(
        FrameGraph& framegraph,
        resources::ResourceRegistry& registry,
        resources::ResourceManager& resources
    );
    ~FrameGraphBuilder() = default;

    template <typename SetupDataType, typename ExecuteDataType>
    void addPass(
        const std::string& name,
        std::function<SetupDataType(PassBuilder&)> setup,
        std::function<void(const ExecuteDataType&, command::RenderCommandBuffer&)> execute
    ) {
      PassBuilder passBuilder;

      SetupDataType passData = setup(passBuilder);

      auto executeFunc = [passData, execute](command::RenderCommandBuffer& cmdBuffer) {
        execute(passData, cmdBuffer);
      };

      m_passes.push_back(
          {name, std::move(executeFunc), passBuilder.getInputs(), passBuilder.getOutputs()}
      );
    }

    auto build() -> FrameGraph;

    auto createTexture(const std::string& name, const resources::TextureDesc& desc)
        -> ::kst::core::RenderResourceHandle;
    auto createBuffer(const std::string& name, const resources::BufferDesc& desc)
        -> ::kst::core::RenderResourceHandle;
    auto importResource(const std::string& name, resource::ResourceID resource)
        -> ::kst::core::RenderResourceHandle;

  private:
    FrameGraph& m_framegraph;
    resources::ResourceRegistry& m_resourceRegistry;
    resources::ResourceManager& m_resourceManager;

    struct PassEntry {
      std::string name;
      std::function<void(command::RenderCommandBuffer&)> executeFunc;
      std::vector<std::string> inputs;
      std::vector<std::string> outputs;
    };

    std::vector<PassEntry> m_passes;
    std::unordered_map<std::string, resources::RenderResource> m_resources;
  };
} // namespace kst::renderer::framegraph
