#include "GraphicsContext.hpp"

#include <algorithm>
#include <cctype>
#include <memory>
#include <string>

#include "core/Logger.hpp"
#include "VulkanBackend/VulkanContext.hpp"

namespace kst::renderer {
  auto GraphicsContext::create(const std::string& backendType, const ContextOptions& options)
      -> std::shared_ptr<GraphicsContext> {
    std::string lowerBackendType = backendType;
    std::ranges::transform(lowerBackendType, lowerBackendType.begin(), [](unsigned char cha) {
      return std::tolower(cha);
    });

    if (lowerBackendType == "vulkan") {
      KST_CORE_INFO("Creating Vulkan rendering context");
      return std::make_shared<VulkanContext>(options);
    }

    KST_CORE_ERROR("Unsupported rendering backend: {}", backendType);
    return nullptr;
  }
} // namespace kst::renderer
