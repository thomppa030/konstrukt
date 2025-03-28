#include <iostream>

#include <gtest/gtest.h>

#include <core/GraphicsType.hpp>

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
  EXPECT_EQ(static_cast<uint8_t>(ResourceState::PRESENT), 15);
}

TEST(GraphicsTypeTest, MemoryDomainEnumValues) {
  // Test memory domain values
  EXPECT_EQ(static_cast<uint8_t>(MemoryDomain::GPU_ONLY), 0);
  EXPECT_EQ(static_cast<uint8_t>(MemoryDomain::CPU_TO_GPU), 1);
  EXPECT_EQ(static_cast<uint8_t>(MemoryDomain::GPU_TO_CPU), 2);
  EXPECT_EQ(static_cast<uint8_t>(MemoryDomain::CPU_AND_GPU), 3);
}

