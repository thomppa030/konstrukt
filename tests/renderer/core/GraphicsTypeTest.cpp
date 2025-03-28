#include <iostream>

#include <gtest/gtest.h>

#include <renderer/core/GraphicsType.hpp>

using namespace kst::core;

TEST(GraphicsTypeTest, BufferUsageFlagsBitwiseOperations) {
  // Test basic flag combinations
  auto combined = BufferUsageFlags::VERTEX_BUFFER | BufferUsageFlags::INDEX_BUFFER;
  EXPECT_EQ(static_cast<uint8_t>(combined), 0b11);

  // Test AND operation
  auto flags1 = BufferUsageFlags::VERTEX_BUFFER | BufferUsageFlags::INDEX_BUFFER;
  auto flags2 = BufferUsageFlags::VERTEX_BUFFER | BufferUsageFlags::UNIFORM_BUFFER;
  auto result = flags1 & flags2;
  EXPECT_EQ(static_cast<uint8_t>(result), static_cast<uint8_t>(BufferUsageFlags::VERTEX_BUFFER));

  // Test |= operator
  BufferUsageFlags flags = BufferUsageFlags::NONE;
  flags |= BufferUsageFlags::VERTEX_BUFFER;
  EXPECT_EQ(static_cast<uint8_t>(flags), static_cast<uint8_t>(BufferUsageFlags::VERTEX_BUFFER));
}

TEST(GraphicsTypeTest, TextureUsageFlagsBitwiseOperations) {
  // Test basic flag combinations
  auto combined = TextureUsageFlags::SAMPLED | TextureUsageFlags::STORAGE;
  EXPECT_EQ(static_cast<uint8_t>(combined), 0b11);

  // Test AND operation
  auto flags1 = TextureUsageFlags::SAMPLED | TextureUsageFlags::STORAGE;
  auto flags2 = TextureUsageFlags::SAMPLED | TextureUsageFlags::COLOR_ATTACHMENT;
  auto result = flags1 & flags2;
  EXPECT_EQ(static_cast<uint8_t>(result), static_cast<uint8_t>(TextureUsageFlags::SAMPLED));

  // Test |= operator
  TextureUsageFlags flags = TextureUsageFlags::NONE;
  flags |= TextureUsageFlags::SAMPLED;
  EXPECT_EQ(static_cast<uint8_t>(flags), static_cast<uint8_t>(TextureUsageFlags::SAMPLED));
}

TEST(GraphicsTypeTest, FormatEnumValues) {
  // Test some key format values
  EXPECT_EQ(static_cast<uint8_t>(Format::UNKNOWN), 0);
  EXPECT_EQ(static_cast<uint8_t>(Format::RGBA8_UNORM), 9);
  EXPECT_EQ(static_cast<uint8_t>(Format::D32_FLOAT), 45);
  EXPECT_EQ(static_cast<uint8_t>(Format::BC7_SRGB), 62);
}

TEST(GraphicsTypeTest, ResourceStateEnumValues) {
  // Test some key resource state values
  EXPECT_EQ(static_cast<uint8_t>(ResourceState::UNDEFINED), 0);
  EXPECT_EQ(static_cast<uint8_t>(ResourceState::GENERAL), 1);
  EXPECT_EQ(static_cast<uint8_t>(ResourceState::RENDER_TARGET), 8);
  EXPECT_EQ(static_cast<uint8_t>(ResourceState::PRESENT), 15);
}

TEST(GraphicsTypeTest, ShaderStageBitwiseOperations) {
  // Test basic flag combinations
  auto combined = ShaderStage::VERTEX | ShaderStage::FRAGMENT;
  EXPECT_EQ(static_cast<uint8_t>(combined), 0b11);

  // Test multiple flags
  auto multistage = ShaderStage::VERTEX | ShaderStage::FRAGMENT | ShaderStage::COMPUTE;
  EXPECT_EQ(static_cast<uint8_t>(multistage), 0b111);

  // Test AND operation
  auto stages1 = ShaderStage::VERTEX | ShaderStage::FRAGMENT | ShaderStage::COMPUTE;
  auto stages2 = ShaderStage::VERTEX | ShaderStage::GEOMETRY;
  auto result  = stages1 & stages2;
  EXPECT_EQ(static_cast<uint8_t>(result), static_cast<uint8_t>(ShaderStage::VERTEX));

  // Test |= operator
  ShaderStage stages = ShaderStage::VERTEX;
  stages |= ShaderStage::FRAGMENT;
  EXPECT_EQ(
      static_cast<uint8_t>(stages),
      static_cast<uint8_t>(ShaderStage::VERTEX | ShaderStage::FRAGMENT)
  );
}

TEST(GraphicsTypeTest, ViewportStructInitialization) {
  // Test default initialization
  Viewport defaultViewport{};
  EXPECT_DOUBLE_EQ(defaultViewport.x, 0.0);
  EXPECT_DOUBLE_EQ(defaultViewport.y, 0.0);
  EXPECT_DOUBLE_EQ(defaultViewport.width, 0.0);
  EXPECT_DOUBLE_EQ(defaultViewport.height, 0.0);
  EXPECT_DOUBLE_EQ(defaultViewport.minDepth, 0.0);
  EXPECT_DOUBLE_EQ(defaultViewport.maxDepth, 1.0);

  // Test custom initialization
  Viewport customViewport{10.0, 20.0, 800.0, 600.0, 0.1, 0.9};
  EXPECT_DOUBLE_EQ(customViewport.x, 10.0);
  EXPECT_DOUBLE_EQ(customViewport.y, 20.0);
  EXPECT_DOUBLE_EQ(customViewport.width, 800.0);
  EXPECT_DOUBLE_EQ(customViewport.height, 600.0);
  EXPECT_DOUBLE_EQ(customViewport.minDepth, 0.1);
  EXPECT_DOUBLE_EQ(customViewport.maxDepth, 0.9);
}

TEST(GraphicsTypeTest, ScissorStructInitialization) {
  // Test default initialization
  Scissor defaultScissor{};
  EXPECT_EQ(defaultScissor.x, 0);
  EXPECT_EQ(defaultScissor.y, 0);
  EXPECT_EQ(defaultScissor.width, 0);
  EXPECT_EQ(defaultScissor.height, 0);

  // Test custom initialization
  Scissor customScissor{10, 20, 800, 600};
  EXPECT_EQ(customScissor.x, 10);
  EXPECT_EQ(customScissor.y, 20);
  EXPECT_EQ(customScissor.width, 800);
  EXPECT_EQ(customScissor.height, 600);
}

TEST(GraphicsTypeTest, ClearValueColorInitialization) {
  ClearValue clearColor{};
  clearColor.color = {0.1f, 0.2f, 0.3f, 1.0f};

  EXPECT_FLOAT_EQ(clearColor.color.r, 0.1f);
  EXPECT_FLOAT_EQ(clearColor.color.g, 0.2f);
  EXPECT_FLOAT_EQ(clearColor.color.b, 0.3f);
  EXPECT_FLOAT_EQ(clearColor.color.a, 1.0f);
}

TEST(GraphicsTypeTest, ClearValueDepthStencilInitialization) {
  ClearValue clearDepthStencil{};
  clearDepthStencil.depthStencil = {1.0f, 0};

  EXPECT_FLOAT_EQ(clearDepthStencil.depthStencil.depth, 1.0f);
  EXPECT_EQ(clearDepthStencil.depthStencil.stencil, 0);
}

TEST(GraphicsTypeTest, TextureRegionInitialization) {
  // Test default initialization
  TextureRegion defaultRegion{};
  EXPECT_EQ(defaultRegion.mipLevel, 0);
  EXPECT_EQ(defaultRegion.arrayLayer, 0);
  EXPECT_EQ(defaultRegion.x, 0);
  EXPECT_EQ(defaultRegion.y, 0);
  EXPECT_EQ(defaultRegion.z, 0);
  EXPECT_EQ(defaultRegion.width, 0);
  EXPECT_EQ(defaultRegion.height, 0);
  EXPECT_EQ(defaultRegion.depth, 0);

  // Test custom initialization
  TextureRegion customRegion{1, 2, 10, 20, 0, 100, 200, 1};
  EXPECT_EQ(customRegion.mipLevel, 1);
  EXPECT_EQ(customRegion.arrayLayer, 2);
  EXPECT_EQ(customRegion.x, 10);
  EXPECT_EQ(customRegion.y, 20);
  EXPECT_EQ(customRegion.z, 0);
  EXPECT_EQ(customRegion.width, 100);
  EXPECT_EQ(customRegion.height, 200);
  EXPECT_EQ(customRegion.depth, 1);
}

TEST(GraphicsTypeTest, EnumValues) {
  // Test FilterMode values
  EXPECT_EQ(static_cast<uint8_t>(FilterMode::NEAREST), 0);
  EXPECT_EQ(static_cast<uint8_t>(FilterMode::LINEAR), 1);
  EXPECT_EQ(static_cast<uint8_t>(FilterMode::ANISOTROPIC), 2);

  // Test AddressMode values
  EXPECT_EQ(static_cast<uint8_t>(AddressMode::REPEAT), 0);
  EXPECT_EQ(static_cast<uint8_t>(AddressMode::MIRRORED_REPEAT), 1);
  EXPECT_EQ(static_cast<uint8_t>(AddressMode::CLAMP_TO_EDGE), 2);

  // Test PrimitiveTopology values
  EXPECT_EQ(static_cast<uint8_t>(PrimitiveTopology::POINT_LIST), 0);
  EXPECT_EQ(static_cast<uint8_t>(PrimitiveTopology::TRIANGLE_LIST), 3);

  // Test CompareOp values
  EXPECT_EQ(static_cast<uint8_t>(CompareOp::NEVER), 0);
  EXPECT_EQ(static_cast<uint8_t>(CompareOp::LESS), 1);
  EXPECT_EQ(static_cast<uint8_t>(CompareOp::ALWAYS), 6);

  // Test CullMode values
  EXPECT_EQ(static_cast<uint8_t>(CullMode::NONE), 0);
  EXPECT_EQ(static_cast<uint8_t>(CullMode::FRONT), 1);
  EXPECT_EQ(static_cast<uint8_t>(CullMode::BACK), 2);
}
