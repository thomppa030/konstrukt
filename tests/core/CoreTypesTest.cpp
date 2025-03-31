#include <iostream>

#include <gtest/gtest.h>

#include <core/CoreTypes.hpp>

using namespace kst::core;

TEST(CoreTypesTest, MemoryDomainEnumValues) {
  // Test memory domain values
  EXPECT_EQ(static_cast<uint8_t>(MemoryDomain::GPU_ONLY), 0);
  EXPECT_EQ(static_cast<uint8_t>(MemoryDomain::CPU_TO_GPU), 1);
  EXPECT_EQ(static_cast<uint8_t>(MemoryDomain::GPU_TO_CPU), 2);
  EXPECT_EQ(static_cast<uint8_t>(MemoryDomain::CPU_AND_GPU), 3);
}

TEST(CoreTypesTest, FeatureFlagBitwiseOperations) {
  // Test basic flag combinations
  auto combined = FeatureFlag::COMPUTE_SHADERS | FeatureFlag::TESSELLATION_SHADERS;
  EXPECT_EQ(static_cast<uint32_t>(combined), 0b11);

  // Test multiple flags
  auto multifeature = FeatureFlag::COMPUTE_SHADERS | FeatureFlag::TESSELLATION_SHADERS |
                      FeatureFlag::GEOMETRY_SHADER;
  EXPECT_EQ(static_cast<uint32_t>(multifeature), 0b111);

  // Test AND operation
  auto features1 = FeatureFlag::COMPUTE_SHADERS | FeatureFlag::TESSELLATION_SHADERS;
  auto features2 = FeatureFlag::COMPUTE_SHADERS | FeatureFlag::MESH_SHADER;
  auto result    = features1 & features2;
  EXPECT_EQ(static_cast<uint32_t>(result), static_cast<uint32_t>(FeatureFlag::COMPUTE_SHADERS));

  // Test |= operator
  FeatureFlag features = FeatureFlag::NONE;
  features |= FeatureFlag::FILL_MODE_NON_SOLID;
  EXPECT_EQ(
      static_cast<uint32_t>(features),
      static_cast<uint32_t>(FeatureFlag::FILL_MODE_NON_SOLID)
  );
}

