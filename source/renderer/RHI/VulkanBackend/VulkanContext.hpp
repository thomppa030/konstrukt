#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include <GLFW/glfw3.h>

#include "../GraphicsContext.hpp"
#include "VulkanCore/Context.hpp"

namespace kst::renderer {
  class VulkanContext : public GraphicsContext {
  public:
    explicit VulkanContext(const ContextOptions& options);

    ~VulkanContext() override;

    VulkanContext(const VulkanContext&)                    = delete;
    VulkanContext(VulkanContext&&)                         = delete;
    auto operator=(const VulkanContext&) -> VulkanContext& = delete;
    auto operator=(VulkanContext&&) -> VulkanContext&      = delete;

    auto createSurface(const SurfaceDescriptor& descriptor) -> bool override;

    auto createBuffer(
        uint64_t size,
        uint64_t usage,
        bool hostVisible        = false,
        const std::string& name = ""
    ) -> std::shared_ptr<Buffer> override;

    auto createTexture(
        uint32_t width,
        uint32_t height,
        uint32_t format,
        uint32_t usage,
        const std::string& name = ""
    ) -> std::shared_ptr<Buffer> override;

    auto createSampler(const std::string& name = "") -> std::shared_ptr<Sampler> override;

    auto createCommandQueue(uint32_t queueType, const std::string& name = "")
        -> std::shared_ptr<CommandQueue> override;

    auto beginFrame() -> uint32_t override;

    void endFrame() override;

    void waitIdle() override;

    auto supportsFeature(uint32_t featureID) const -> bool override;

    auto getImplementationName() const -> const char* override { return "Vulkan"; }

  private:
    void initVulkan(const ContextOptions& options);
    static void
    setupInstanceExtension(std::vector<std::string>& extensions, const ContextOptions& options);
    static void
    setupDeviceExtension(std::vector<std::string>& extensions, const ContextOptions& options);
    static void
    setupValidationLayers(std::vector<std::string>& layers, const ContextOptions& options);

    std::shared_ptr<VulkanCore::Context> m_context;

    uint32_t m_currenFrame = 0;

    std::unordered_map<uint32_t, std::shared_ptr<CommandQueue>> m_commandQueues;
  };
} // namespace kst::renderer
