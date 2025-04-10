#pragma once

#include <array>
#include <cstdint>
#include <limits>

#include <core/CoreTypes.hpp>

namespace kst::core {

  /**
   *  Pixel formats for textures, render targets and other image resources
   */
  enum class Format : std::uint8_t {
    UNKNOWN,
    R8_UNORM,   // 8-bit red channel (normalized)
    R8_SNORM,   // 8-bit red channel (signed normalized)
    R8_UINT,    // 8-bit red channel (unsigned integer)
    R8_SINT,    // 8-bit red channel (signed integer)

    R8G8_UNORM, // 8-bit red and green channels
    R8G8_SNORM,
    R8G8_UINT,
    R8G8_SINT,

    RGBA8_UNORM, // 8-bit RGBA (standard 32-bit color)
    RGBA8_SNORM,
    RGBA8_UINT,
    RGBA8_SINT,
    RGBA8_SRGB,  // sRGB color space (gamma corrected)

    BGRA8_UNORM, // 8-bit BGRA (common for platform-specific textures)
    BGRA8_SRGB,

    // Higher precision formats
    R16_FLOAT, // 16-bit floating point
    R16_UINT,
    R16_SINT,
    R16_UNORM,
    R16_SNORM,

    RG16_FLOAT,
    RG16_UINT,
    RG16_SINT,
    RG16_UNORM,
    RG16_SNORM,

    RGBA16_FLOAT,
    RGBA16_UINT,
    RGBA16_SINT,
    RGBA16_UNORM,
    RGBA16_SNORM,

    R32_FLOAT, // 32-bit floating point
    R32_UINT,
    R32_SINT,

    RG32_FLOAT,
    RG32_UINT,
    RG32_SINT,

    RGB32_FLOAT,
    RGB32_UINT,
    RGB32_SINT,

    RGBA32_FLOAT,
    RGBA32_UINT,
    RGBA32_SINT,

    // Depth / stencil formats
    D16_UNORM,         // 16-bit depth
    D24_UNORM_S8_UINT, // 24-bit depth, 8-bit Stencil
    D32_FLOAT,         // 32-bit depth
    D32_FLOAT_S8_UINT, // 32-bit depth, 8-bit Stencil

    // compressed formats
    BCA1_RGB_UNORM, // Block Compression 1 (DXT1)
    BCA1_RGB_SRGB,
    BCA1_RGBA_UNORM,
    BCA1_RGBA_SRGB,
    BC2_UNORM, // Block Compression 2 (DXT3)
    BC2_SRGB,
    BC3_UNORM, // Block Compression 3 (DXT5)
    BC3_SRGB,
    BC4_UNORM, // Block Compression 4 (single channel)
    BC4_SNORM,
    BC5_UNORM, // Block Compression 5 (two channels)
    BC5_SNORM,
    BC6H_UF16, // Block Compression 6 (HDR)
    BC6H_SF16,
    BC7_UNORM, // Block Compression 7 (high quality)
    BC7_SRGB
  };

  /*
   *  Describes what kind of Resource
   */
  enum class ResourceType : std::uint8_t {
    UNKNOWN, // Unknown or invalid resource type
    BUFFER,
    VERTEX_BUFFER,
    INDEX_BUFFER,
    UNIFORM_BUFFER,
    STORAGE_BUFFER,
    TEXTURE,
    RENDER_TARGET,
    DEPTH_STENCIL,
    BINDLESS_TABLE,
    MESH,     // Mesh resource containing geometry data
    MATERIAL, // Material resource containing rendering properties
    MODEL
  };

  /**
   *  Describes how a resource can be used by the GPU
   *  Can have multiple usage flags combined with bitwise OR
   */
  enum class BufferUsageFlags : std::uint8_t {
    NONE              = 0,
    VERTEX_BUFFER     = 1U << 0U, // Buffer can be used as a vertex buffer
    INDEX_BUFFER      = 1U << 1U, // Buffer can be used as an index buffer
    UNIFORM_BUFFER    = 1U << 2U, // Buffer can be used as a uniform buffer
    STORAGE_BUFFER    = 1U << 3U, // Buffer can be used as a storage buffer
    INDIRECT_BUFFER   = 1U << 4U, // Buffer can be used for indirect draw commands
    TRANSFER_SRC      = 1U << 5U, // Buffer can be used as a source for copy commands
    TRANSFER_DST      = 1U << 6U, // Buffer can be used as a destination for copy commands
    RAYTRACING_BUFFER = 1U << 7U, // Buffer can be used for ray tracing acceleration structures
  };

  inline auto operator|(BufferUsageFlags flagA, BufferUsageFlags flagB) -> BufferUsageFlags {
    return static_cast<BufferUsageFlags>(
        static_cast<std::uint8_t>(flagA) | static_cast<std::uint8_t>(flagB)
    );
  }

  inline auto operator&(BufferUsageFlags flagA, BufferUsageFlags flagB) -> BufferUsageFlags {
    return static_cast<BufferUsageFlags>(
        static_cast<std::uint8_t>(flagA) & static_cast<std::uint8_t>(flagB)
    );
  }

  inline auto operator|=(BufferUsageFlags& flagA, const BufferUsageFlags& flagB)
      -> BufferUsageFlags& {
    return flagA = flagA | flagB;
  }

  /**
   *  Describes how a texture can be used by the GPU
   *  Can have multiple usage flags combined with bitwise OR
   */
  enum class TextureUsageFlags : std::uint8_t {
    NONE             = 0,
    SAMPLED          = 1U << 0U, // Texture can be sampled in a shader
    STORAGE          = 1U << 1U, // Texture can be used as a storage image
    COLOR_ATTACHMENT = 1U << 2U, // Texture can be used as a color attachment
    DEPTH_STENCIL    = 1U << 3U, // Texture can be used as a depth/stencil attachment
    TRANSFER_SRC     = 1U << 4U, // Texture can be used as a source for copy commands
    TRANSFER_DST     = 1U << 5U, // Texture can be used as a destination for copy commands
    INPUT_ATTACHMENT = 1U << 6U, // Texture can be used as an input attachment
  };

  inline auto operator|(TextureUsageFlags flagA, TextureUsageFlags flagB) -> TextureUsageFlags {
    return static_cast<TextureUsageFlags>(
        static_cast<std::uint8_t>(flagA) | static_cast<std::uint8_t>(flagB)
    );
  }

  inline auto operator&(TextureUsageFlags flagA, TextureUsageFlags flagB) -> TextureUsageFlags {
    return static_cast<TextureUsageFlags>(
        static_cast<std::uint8_t>(flagA) & static_cast<std::uint8_t>(flagB)
    );
  }

  inline auto operator|=(TextureUsageFlags& flagA, const TextureUsageFlags& flagB)
      -> TextureUsageFlags& {
    return flagA = flagA | flagB;
  }

  /**
   *  Resource states describe the current access pattern and layout of a resource
   *  State transitions are necessary for synchronization between GPU operations
   */
  enum class ResourceState : std::uint8_t {
    UNDEFINED,
    GENERAL,             // General state, usable for every operation

    VERTEX_BUFFER,       // Buffer is used as a vertex buffer
    INDEX_BUFFER,        // Buffer is used as an index buffer
    CONSTANT_BUFFER,     // Buffer is used as a constant/uniform Buffer
    INDIRECT_BUFFER,     // Buffer is used for indirect draw commands
    SHADER_RESOURCE,     // Buffer is used as a shader resource
    UNORDERED_ACCESS,    // Buffer is used as an unordered access resource

    RENDER_TARGET,       // Texture is used as a render target
    DEPTH_STENCIL_READ,  // Texture is used a read-only depth/stencil buffer
    DEPTH_STENCIL_WRITE, // Texture is used as a write-enabled depth/stencil buffer
    SHADER_READ,         // Texture is used for shader reading
    SHADER_WRITE,        // Texture is used for shader writing

    COPY_SOURCE,         // Resource is used as a source for a copy operation
    COPY_DESTINATION,    // Resource is used as a destination for a copy operation

    // Present states
    PRESENT
  };

  /**
   * Filtering Modes for texture sampling
   */
  enum class FilterMode : std::uint8_t {
    NEAREST,    // Nearest-neighbor filtering
    LINEAR,     // Linear filtering (smooth)
    ANISOTROPIC // Anisotropic filtering (high quality)
  };

  /*
   * Address Modes for texture coordinate wrapping
   */
  enum class AddressMode : std::uint8_t {
    REPEAT,               // Repeat the texture
    MIRRORED_REPEAT,      // Repeat the texture mirrored
    CLAMP_TO_EDGE,        // Clamp texture coordinates to edge
    CLAMP_TO_BORDER,      // Clamp texture coordinates to a specified border color
    MIRROR_CLAMP_TO_EDGE, // Mirrors once, then clamps to edge
  };

  /*
   *  Primitivs topology types for rendering geometry in graphics pipelines
   */
  enum class PrimitiveTopology : std::uint8_t {
    POINT_LIST,     // Renders points
    LINE_LIST,      // Renders lines (pairs of vertices)
    LINE_STRIP,     // Renders connected lines
    TRIANGLE_LIST,  // Renders triangles (triplets of vertices)
    TRIANGLE_STRIP, // Renders connected triangles
    TRIANGLE_FAN,   // Renders triangles connected by a single vertex
    PATCH_LIST      // Renders tessellated control points
  };

  /*
   *  Compare operations for depth and stencil operations
   */
  enum class CompareOp : std::uint8_t {
    NEVER,            // Never passes the test
    LESS,             // Passes if new value < existing value
    EQUAL,            // Passes if new value == existing value
    LESS_OR_EQUAL,    // Passes if new value <= existing value
    GREATER,          // Passes if new value > existing value
    GREATER_OR_EQUAL, // Passes if new value >= existing value
    ALWAYS,           // Always passes the test
  };

  /**
   * Blend factors for color blending operations
   */
  enum class BlendFactor : std::uint8_t {
    ZERO,                     // factor is 0
    ONE,                      // factor is 1
    SRC_COLOR,                // factor is the source color
    ONE_MINUS_SRC_COLOR,      // factor is 1 - source color
    DST_COLOR,                // factor is the destination color
    ONE_MINUS_DST_COLOR,      // factor is 1 - destination color
    SRC_ALPHA,                // factor is the source alpha
    ONE_MINUS_SRC_ALPHA,      // factor is 1 - source alpha
    DST_ALPHA,                // factor is the destination SRC_ALPHA
    ONE_MINUS_DST_ALPHA,      // factor is 1 - destination DST_ALPHA
    CONSTANT_COLOR,           // factor is a constant color
    ONE_MINUS_CONSTANT_COLOR, // factor is 1 - constant color
    CONSTANT_ALPHA,           // factor is a constant alpha
    ONE_MINUS_CONSTANT_ALPHA, // factor is 1 - constant alpha
    SRC_ALPHA_SATURATE,       // factor is the min(src alpha, 1 - dst alpha)
  };

  /**
   * Blend operations for color blending operations
   */
  enum class BlendOp : std::uint8_t {
    ADD,              // Result = src color + dst color
    SUBTRACT,         // Result = src color - dst color
    REVERSE_SUBTRACT, // Result = dst color - src color
    MIN,              // Result = min(src color, dst color)
    MAX               // Result = max(src color, dst color)
  };

  /*
   *  Shader stages in the graphics pipeline
   */
  enum class ShaderStage : std::uint8_t {
    VERTEX                 = 1U << 0U, // Processes each incoming vertex
    FRAGMENT               = 1U << 1U, // Processes each fragment/pixel
    COMPUTE                = 1U << 2U, // General purpose GPU computation
    GEOMETRY               = 1U << 3U, // Optional stage, that can generate/modify geometry
    TESSELATION_CONTROL    = 1U << 4U, // Controls tessellation of patches
    TESSELATION_EVALUATION = 1U << 5U, // Evaluates tessellated control points
    MESH                   = 1U << 6U, // Creates Mesh geometry (for mesh shaders)
    TASK                   = 1U << 7U  // Dispatches mesh shader workgroups
  };

  inline auto operator|(ShaderStage stageA, const ShaderStage stageB) -> ShaderStage {
    return static_cast<ShaderStage>(
        static_cast<std::uint8_t>(stageA) | static_cast<std::uint8_t>(stageB)
    );
  }

  inline auto operator&(ShaderStage stageA, const ShaderStage stageB) -> ShaderStage {
    return static_cast<ShaderStage>(
        static_cast<std::uint8_t>(stageA) & static_cast<std::uint8_t>(stageB)
    );
  }

  inline auto operator|=(ShaderStage& stageA, const ShaderStage& stageB) -> ShaderStage& {
    return stageA = stageA | stageB;
  }

  /*
   *  Cull modes for polygon culling
   */
  enum class CullMode : std::uint8_t {
    NONE,          // Culling disabled
    FRONT,         // Cull front-facing polygons
    BACK,          // Cull back-facing polygons
    FRONT_AND_BACK // Cull both front- and back-facing polygons
  };

  /*
   *  Fill modes for rasterization
   */
  enum class FillMode : std::uint8_t {
    SOLID,      // Fills polygons with a solid color
    WIRFEFRAME, // Fills polygons with a wireframe
    POINT       // Renders points at each vertex
  };

  /*
   *  Logic operations for framebuffer blending
   */
  enum class LogicOp : std::uint8_t {
    CLEAR,         // 0
    AND,           // s & d
    AND_REVERSE,   // s &~d
    COPY,          // s
    AND_INVERTED,  // ~s & d
    NO_OP,         // d
    XOR,           // s ^ d
    OR,            // s | d
    NOR,           // ~(s | d)
    EQUIVALENT,    // ~(s ^ d)
    INVERT,        // ~d
    OR_REVERSE,    // s | ~d
    COPY_INVERTED, // ~s
    OR_INVERTED,   // ~s | d
    NAND,          // ~(s & d)
    SET            // 1
  };

  /*
   *  Stencil operations for stencil testing
   */
  enum class StendilOp : std::uint8_t {
    KEEP,                // Keep the current value
    ZERO,                // Set to 0
    REPLACE,             // Replace with reference value
    INCREMENT_AND_CLAMP, // Increment and clamp
    DECREMENT_AND_CLAMP, // Decrement and clamp
    INVERT,              // Bitwise invert
    INCREMENT_WRAP,      // Increment and wrap around
    DECREMENT_WRAP       // Decrement and wrap around
  };

  /*
   *  Types of Objects for debugging and labeling
   */
  enum class ObjectType : std::uint8_t {
    BUFFER,
    TEXTURE,
    SHADER,
    PIPELINE,
    RENDER_PASS,
    FRAMEBUFFER,
    COMMAND_BUFFER,
    DESCRIPTOR_SET,
    SAMPLER,
    QUERY
  };

  /*
   *  Viewport definition for rendering
   */
  struct Viewport {
    double x{};
    double y{};
    double width{};
    double height{};
    double minDepth = 0.0;
    double maxDepth = 1.0;
  };

  struct Scissor {
    int32_t x{};
    int32_t y{};
    int32_t width{};
    int32_t height{};
  };

  /*
   *  Clear Values for render targets.
   */
  struct ClearValue {
    struct ColorValue {
      float r, g, b, a;
    };

    struct DepthStencil {
      float depth;
      uint8_t stencil;
    };
    union {
      ColorValue color;
      DepthStencil depthStencil;
    };
  };

  /*
   *  Defines a region of a texture.
   */
  struct TextureRegion {
    uint32_t mipLevel   = 0;
    uint32_t arrayLayer = 0;
    uint32_t x{};
    uint32_t y{};
    uint32_t z{};
    uint32_t width{};
    uint32_t height{};
    uint32_t depth{};
  };

  /*
   *  Device Limits and capabilities.
   */
  struct Limits {
    uint32_t maxImageDimension1D                   = 0;
    uint32_t maxImageDimension2D                   = 0;
    uint32_t maxImageDimension3D                   = 0;
    uint32_t maxImageDimensionCube                 = 0;
    uint32_t maxImageArrayLayers                   = 0;
    uint32_t maxTexelBufferElements                = 0;
    uint32_t maxUniformBufferRange                 = 0;
    uint32_t maxStorageBufferRange                 = 0;
    uint32_t maxPushConstantsSize                  = 0;
    uint32_t maxMemoryAllocationCount              = 0;
    uint32_t maxSamplerAllocationCount             = 0;
    uint32_t maxBoundDescriptorSets                = 0;
    uint32_t maxPerStageDescriptorSamplers         = 0;
    uint32_t maxPerStageDescriptorUniformBuffers   = 0;
    uint32_t maxPerStageDescriptorStorageBuffers   = 0;
    uint32_t maxPerStageDescriptorSampledImages    = 0;
    uint32_t maxPerStageDescriptorStorageImages    = 0;
    uint32_t maxPerStageResources                  = 0;
    uint32_t maxDescriptorSetSamplers              = 0;
    uint32_t maxDescriptorSetUniformBuffers        = 0;
    uint32_t maxDescriptorSetUniformBuffersDynamic = 0;
    uint32_t maxDescriptorSetStorageBuffers        = 0;
    uint32_t maxDescriptorSetStorageBuffersDynamic = 0;
    uint32_t maxDescriptorSetSampledImages         = 0;
    uint32_t maxDescriptorSetStorageImages         = 0;
    uint32_t maxFramebufferWidth                   = 0;
    uint32_t maxFramebufferHeight                  = 0;
    uint32_t maxFramebufferLayers                  = 0;
    uint32_t maxColorAttachments                   = 0;
    float maxSamplerAnisotropy                     = 0.0F;
  };

  enum class DeviceType : std::uint8_t {
    DISCRETE,
    INTEGRATED,
    SOFTWARE
  };

  namespace memory {
    constexpr uint32_t MAX_MEMORY_HEAPS = 16;
    constexpr uint32_t MAX_MEMORY_TYPES = 32;
  } // namespace memory

  /*
   *  Memory properties of the device.
   */
  struct MemoryProperties {
    uint32_t memoryHeapCount{};
    struct MemoryHeap {
      uint64_t size;
      bool isDeviceLocal;
    };
    std::array<MemoryHeap, memory::MAX_MEMORY_HEAPS> memoryHeaps{};

    uint32_t memoryTypeCount{};
    struct MemoryType {
      uint32_t heapIndex;
      bool isDeviceLocal;
      bool isHostVisible;
      bool isHostCoherent;
      bool isHostCached;
    };
    std::array<MemoryType, memory::MAX_MEMORY_TYPES> memoryTypes{};
  };

} // namespace kst::core
