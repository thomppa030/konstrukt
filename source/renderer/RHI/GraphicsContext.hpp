#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace kst::renderer {
  class Buffer;
  class Sampler;
  class Texture;
  class Pipeline;
  class CommandQueue;
  class RenderPass;
  class FrameBuffer;

  struct SurfaceDescriptor {
    void* nativeWindowHandle = nullptr;
    uint32_t width           = 0;
    uint32_t height          = 0;
  };

  struct ContextOptions {
    bool enableValidation  = true;
    bool enableRayTracing  = true;
    bool debugName         = true;
    bool printEnumerations = true;
    void* window           = nullptr;  // Window handle (e.g., GLFWwindow*)
    uint32_t width         = 0;        // Window width
    uint32_t height        = 0;        // Window height
  };

  class GraphicsContext {
  public:
    virtual ~GraphicsContext() = default;

    virtual auto createSurface(const SurfaceDescriptor& descriptor) -> bool = 0;

    virtual auto createBuffer(
        uint64_t size,
        uint64_t usage,
        bool hostVisible        = false,
        const std::string& name = ""
    ) -> std::shared_ptr<Buffer> = 0;

    virtual auto createTexture(
        uint32_t width,
        uint32_t height,
        uint32_t format,
        uint32_t usage,
        const std::string& name = ""
    ) -> std::shared_ptr<Buffer> = 0;

    virtual auto createSampler(const std::string& name = "") -> std::shared_ptr<Sampler> = 0;

    virtual auto createCommandQueue(uint32_t queueType, const std::string& name = "")
        -> std::shared_ptr<CommandQueue> = 0;

    virtual auto beginFrame() -> uint32_t = 0;

    virtual void endFrame() = 0;

    virtual void waitIdle() = 0;

    virtual auto supportsFeature(uint32_t featureID) const -> bool = 0;

    virtual auto getImplementationName() const -> const char* = 0;

    static auto create(const std::string& backendType, const ContextOptions& options = {})
        -> std::shared_ptr<GraphicsContext>;
  };
} // namespace kst::renderer
