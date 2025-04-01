#include "FramegraphBuilder.hpp"

#include <string>

#include <boost/range/algorithm/find.hpp>

#include "renderer/framegraph/Framegraph.hpp"
#include "renderer/framegraph/RenderPass.hpp"
#include "renderer/resources/RenderResource.hpp"
#include "renderer/resources/ResourceID.hpp"
#include "renderer/resources/ResourceManager.hpp"
#include "renderer/resources/ResourceRegistry.hpp"

namespace kst::renderer::framegraph {

  FrameGraphBuilder::FrameGraphBuilder(
      FrameGraph& framegraph,
      resources::ResourceRegistry& registry,
      resources::ResourceManager& resources
  )
      : m_framegraph(framegraph), m_resourceRegistry(registry), m_resourceManager(resources) {}
  auto FrameGraphBuilder::build() -> FrameGraph {
    for (const auto& passEntry : m_passes) {
      RenderPass pass;
      pass.setName(passEntry.name);

      for (const auto& input : passEntry.inputs) {
        if (auto* resource = m_framegraph.getResource(input)) {
          resource->addConsumer(&pass);
        }
      }

      for (const auto& output : passEntry.outputs) {
        pass.addOutput(output);

        if (auto* resource = m_framegraph.getResource(output)) {
          resource->setProducer(&pass);
        }
      }

      pass.setExecuteFunction(passEntry.executeFunc);

      m_framegraph.addPass(pass);
    }

    for (const auto& [name, resource] : m_resources) {
      m_framegraph.addResource(name, resource);
    }

    m_framegraph.compile();

    return m_framegraph;
  }

  auto FrameGraphBuilder::createTexture(const std::string& name, const resources::TextureDesc& desc)
      -> ::kst::core::RenderResourceHandle {
    auto resDesc = resources::ResourceDesc(::kst::core::ResourceType::TEXTURE, desc);

    // Create the actual GPU texture through ResourceManager
    auto resourceId = m_resourceManager.createTexture(desc);

    // Store the resource in our local map with its description
    m_resources[name] = resources::RenderResource(::kst::core::ResourceType::TEXTURE, resourceId);

    return name;
  }

  auto FrameGraphBuilder::createBuffer(const std::string& name, const resources::BufferDesc& desc)
      -> ::kst::core::RenderResourceHandle {
    auto resDesc = resources::ResourceDesc(::kst::core::ResourceType::BUFFER, desc);

    // Store the resource in our local map with its description
    m_resources[name] = resources::RenderResource(resDesc);

    return name;
  }

  auto FrameGraphBuilder::importResource(const std::string& name, resource::ResourceID resource)
      -> ::kst::core::RenderResourceHandle {
    // Get the resource type from the registry
    auto resourceType = m_resourceRegistry.getResourceType(resource);

    // Try to get the detailed resource description from the resource manager
    const auto* desc = m_resourceManager.getResourceDesc(resource);

    if (desc != nullptr) {
      // If we have a full description, use it
      m_resources[name] = resources::RenderResource(*desc);
    } else {
      // Otherwise just use the basic type and ID
      m_resources[name] = resources::RenderResource(resourceType, resource);
    }

    return name;
  }

  void PassBuilder::read(const std::string& resourceName) {
    if (boost::range::find(m_outputs, resourceName) == m_outputs.end()) {
      m_inputs.push_back(resourceName);
    }
  }

  void PassBuilder::write(const std::string& resourceName) {
    if (boost::range::find(m_outputs, resourceName) == m_outputs.end()) {
      m_outputs.push_back(resourceName);
    }
  }

} // namespace kst::renderer::framegraph
