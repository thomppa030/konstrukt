#include "RenderResource.hpp"

#include "core/log/Logger.hpp"
#include "renderer/framegraph/RenderPass.hpp"

namespace kst::renderer::resources {

  RenderResource::RenderResource(const ResourceDesc& desc)
      : m_type(desc.type), m_state(desc.initialState), m_resourceDesc(desc),
        m_transient(desc.transient) {}

  RenderResource::RenderResource(
      core::ResourceType type,
      resource::ResourceID rID,
      core::ResourceState initialState
  )
      : m_type(type), m_state(initialState), m_resourceID(rID) {}

  RenderResource::RenderResource(core::ResourceType type, uint32_t bindlessIndex)
      : m_type(type), m_bindlessIndex(bindlessIndex) {}

  auto RenderResource::getName() const -> const std::string& {
    return m_name;
  }

  void RenderResource::addConsumer(framegraph::RenderPass* pass) {
    if (pass == nullptr) {
      kst::core::Logger::warn<>("Attempted to add null pass as consumer");
      return;
    }

    // Check if the pass is already a consumer
    for (auto* consumer : m_consumers) {
      if (consumer == pass) {
        return; // Already added
      }
    }

    m_consumers.push_back(pass);
  }

} // namespace kst::renderer::resources

