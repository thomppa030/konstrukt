#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "RenderPass.hpp"
#include "renderer/core/GraphicsContext.hpp"
#include "renderer/resources/RenderResource.hpp"

namespace kst::renderer::framegraph {

  class ResourceDesc;
  class RenderAPI;

  /*
   * The FrameGraph represents the entire rendering process as a directed acyclic graph:
   *   - Manages the collection of passes and resource
   *   - Analyzes the dependencies between passes
   *   - Optimizes resource allocation and pass execution
   *   - Handles synchronization between passes
   * */
  class FrameGraph {
  public:
    FrameGraph()  = default;
    ~FrameGraph() = default;

    /**
     * @brief Copy constructor for deep-copying a FrameGraph
     *
     * Creates a complete deep copy of the source FrameGraph, including all
     * render passes and resources. This is useful for caching a pre-built
     * graph structure that can be reused or modified later.
     *
     * Note: Based on the current FrameGraphBuilder implementation, which modifies
     * an existing FrameGraph, this constructor may not be directly used but is
     * provided for completeness and potential future use cases.
     *
     * @param other The source FrameGraph to copy
     */
    FrameGraph(const FrameGraph& other) : m_passes(other.m_passes), m_resources(other.m_resources) {
      // Update pass references within resources
      for (auto& [name, resource] : m_resources) {
        // Reset producer and consumers since we can't directly copy the pointers
        resource.setProducer(nullptr);

        // Find the original producer in the source graph
        auto* originalProducer = other.m_resources.at(name).getProducer();
        if (originalProducer != nullptr) {
          // Find the corresponding pass in our new copy
          for (auto& pass : m_passes) {
            if (pass.getName() == originalProducer->getName()) {
              resource.setProducer(&pass);
              break;
            }
          }
        }

        // Clear and rebuild consumers list with correct pointers
        const auto& originalConsumers = other.m_resources.at(name).getConsumers();
        for (auto* originalConsumer : originalConsumers) {
          // Find matching pass in our copy
          for (auto& pass : m_passes) {
            if (pass.getName() == originalConsumer->getName()) {
              resource.addConsumer(&pass);
              break;
            }
          }
        }
      }
    }

    /**
     * @brief Move constructor for FrameGraph
     *
     * Efficiently transfers ownership of passes and resources from one FrameGraph
     * to another. This is potentially useful when a builder needs to construct and
     * return a new FrameGraph instance.
     *
     * Note: The current FrameGraphBuilder implementation modifies an existing
     * FrameGraph reference rather than constructing and moving a new one.
     * This constructor is provided for completeness and potential future use cases.
     *
     * @param other The source FrameGraph to move from
     */
    FrameGraph(FrameGraph&& other) noexcept
        : m_passes(std::move(other.m_passes)), m_resources(std::move(other.m_resources)) {
      // No need to update pointers as they remain valid after a move
    }

    auto operator=(const FrameGraph&) -> FrameGraph& = default;
    auto operator=(FrameGraph&&) -> FrameGraph&      = default;

    void addPass(const RenderPass& pass);
    auto createResource(std::string& name, const resources::ResourceDesc& desc)
        -> ::kst::core::RenderResourceHandle;
    void addResource(const std::string& name, const resources::RenderResource& resource);

    void compile();
    void execute(core::GraphicsContext& backend);

    auto getResource(const std::string& name) -> resources::RenderResource*;

  private:
    std::vector<RenderPass> m_passes;
    std::unordered_map<std::string, resources::RenderResource> m_resources;
  };
} // namespace kst::renderer::framegraph
