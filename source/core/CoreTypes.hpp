#pragma once

#include <cstdint>

namespace kst::core {

  enum class MemoryDomain : std::uint8_t {
    GPU_ONLY,    // Memory only accessible by the GPU, typically faster
    CPU_TO_GPU,  // Memory for uploading to the GPU (CPU writes, GPU reads)
    GPU_TO_CPU,  // Memory for downloading from the GPU (GPU writes, CPU reads)
    CPU_AND_GPU, // Memory accessible by both CPU and GPU, typically slower
  };

  enum class FeatureFlag : std::uint32_t {
    NONE                     = 0,         // No features supported
    COMPUTE_SHADERS          = 1U << 0U,  // Compute shaders supported
    TESSELLATION_SHADERS     = 1U << 1U,  // Tessellation shaders supported
    GEOMETRY_SHADER          = 1U << 2U,  // Geometry shaders supported
    MESH_SHADER              = 1U << 3U,  // Mesh shaders supported
    TASK_SHADER              = 1U << 4U,  // Task shaders supported
    VARIABLE_RATE_SHADING    = 1U << 5U,  // Variable rate shading supported
    MULTI_VIEWPORT_SUPPORT   = 1U << 6U,  // Multiple viewports supported
    SAMPLER_ANISOTROPY       = 1U << 7U,  // Anisotropic sampler filtering supported
    TEXTURE_COMPRESSION_BC   = 1U << 8U,  // Texture compression BC supported
    TEXTURE_COMPRESSION_ASTC = 1U << 9U,  // Texture compression ASTC supported
    TEXTURE_COMPRESSION_ETC2 = 1U << 10U, // Texture compression ETC2 supported
    BINDLESS_RESOURCES       = 1U << 11U, // Bindless resource support supported
    RAY_TRACING              = 1U << 12U, // Ray tracing supported
    SHADER_INT64             = 1U << 13U, // 64-bit integer shader variables supported
    SHADER_INT16             = 1U << 14U, // 16-bit integer shader variables supported
    SHADER_FLOAT16           = 1U << 15U, // 16-bit floating-point shader variables supported
    DESCRIPTOR_INDEXING      = 1U << 16U, // Descriptor indexing supported
    DRAW_INDIRECT            = 1U << 17U, // Draw indirect supported
    DRAW_INDIRECT_COUNT      = 1U << 18U
  };

  inline auto operator|(FeatureFlag flagA, const FeatureFlag flagB) -> FeatureFlag {
    return static_cast<FeatureFlag>(
        static_cast<std::uint32_t>(flagA) | static_cast<std::uint32_t>(flagB)
    );
  }

  inline auto operator&(FeatureFlag flagA, const FeatureFlag flagB) -> FeatureFlag {
    return static_cast<FeatureFlag>(
        static_cast<std::uint32_t>(flagA) & static_cast<std::uint32_t>(flagB)
    );
  }

  inline auto operator|=(FeatureFlag& flagA, const FeatureFlag& flagB) -> FeatureFlag& {
    return flagA = flagA | flagB;
  }

} // namespace kst::core 