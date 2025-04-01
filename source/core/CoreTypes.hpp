#pragma once

#include <cstdint>
#include <string>

namespace kst::core {

  using RenderResourceHandle = std::string;

  enum class MemoryDomain : std::uint8_t {
    GPU_ONLY,    // Memory only accessible by the GPU, typically faster
    CPU_TO_GPU,  // Memory for uploading to the GPU (CPU writes, GPU reads)
    GPU_TO_CPU,  // Memory for downloading from the GPU (GPU writes, CPU reads)
    CPU_AND_GPU, // Memory accessible by both CPU and GPU, typically slower
  };

  enum class FeatureFlag : std::uint32_t {
    NONE                 = 0,              // No features supported
    COMPUTE_SHADERS      = 1U << 0U,       // Compute shaders supported
    TESSELLATION_SHADERS = 1U << 1U,       // Tessellation shaders supported
    GEOMETRY_SHADER      = 1U << 2U,       // Geometry shaders supported
    MESH_SHADER          = 1U << 3U,       // Mesh shaders supported

    SHADER_FLOAT64            = 1U << 4U,  // 64-bit floating-point shader variables supported
    SHADER_INT64              = 1U << 5U,  // 64-bit integer shader variables supported
    SHADER_INT16              = 1U << 6U,  // 16-bit integer shader variables supported
    SHADER_RESOURCE_RESIDENCY = 1U << 7U,  // Shader resource residency supported
    SHADER_RESOURCE_MIN_LOD   = 1U << 8U,  // Shader resource min/max LOD supported
    SHADER_CLIP_DISTANCE      = 1U << 9U,  // Shader clip distance supported
    SHADER_CULL_DISTANCE      = 1U << 10U, // Shader cull distance supported
    SHADER_STORES_AND_ATOMICS = 1U << 11U, // Shader stores and atomics supported

    // Texture and sampling features
    TEXTURE_CUBE_ARRAY       = 1U << 12U, // Cube map arrays supported
    SAMPLER_ANISOTROPY       = 1U << 13U, // Anisotropic sampler filtering supported
    TEXTURE_COMPRESSION_BC   = 1U << 14U, // Texture compression BC supported
    TEXTURE_COMPRESSION_ASTC = 1U << 15U, // Texture compression ASTC supported
    TEXTURE_COMPRESSION_ETC2 = 1U << 16U, // Texture compression ETC2 supported

    // Rendering features
    MULTI_VIEWPORT_SUPPORT = 1U << 17U,       // Multiple viewports supported
    DEPTH_CLAMPING         = 1U << 18U,       // Depth clamping supported
    DEPTH_BIAS_CLAMP       = 1U << 19U,       // Depth bias supported
    DEPTH_BOUNDS           = 1U << 20U,       // Depth bounds supported

    WIDE_LINES          = 1U << 21U,          // Wide lines supported
    FILL_MODE_NON_SOLID = 1U << 22U,          // Fill mode non-solid supported
    INDEPENDENT_BLEND   = 1U << 23U,          // Independent blend supported
    DUAL_SRC_BLEND      = 1U << 24U,          // Dual source blend supported
    LOGIC_OP            = 1U << 25U,          // Logic operations supported
    SAMPLE_RATE_SHADING = 1U << 26U,          // Sample rate shading supported

    FULL_DRAW_INDEX_UINT32       = 1U << 27U, // Full draw index 32-bit supported
    MULTI_DRAW_INDIRECT          = 1U << 28U, // Multi draw indirect supported
    DRAW_INDIRECT_FIRST_INSTANCE = 1U << 29U, // Draw indirect first instance supported

    // Query features
    OCCLUSION_QUERY_PRECISE   = 1U << 30U, // Precise occlusion query supported
    PIPELINE_STATISTICS_QUERY = 1U << 31U, // Pipeline statistics query supported
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
