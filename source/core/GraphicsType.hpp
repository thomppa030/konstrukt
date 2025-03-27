#pragma once

#include <cstdint>

namespace kst::core {

  /**
   *  Pixel formats for textures, render targets and other image resources
   */
  enum Format : std::uint8_t {
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
} // namespace kst::core
