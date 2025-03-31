#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <renderer/resources/ResourceID.hpp>

using namespace kst::renderer::resource;

// Basic tests for ResourceID functionality
TEST(ResourceIDTest, DefaultConstructor) {
  ResourceID id;
  EXPECT_FALSE(id.isValid());
  EXPECT_EQ(id.index, ResourceID::INVALID_INDEX);
  EXPECT_EQ(id.generation, 0);
}

TEST(ResourceIDTest, InvalidConstruction) {
  ResourceID id = ResourceID::invalid();
  EXPECT_FALSE(id.isValid());
  EXPECT_EQ(id.index, ResourceID::INVALID_INDEX);
  EXPECT_EQ(id.generation, 0);
  
  // Test bool conversion
  EXPECT_FALSE(static_cast<bool>(id));
}

TEST(ResourceIDTest, ValidConstruction) {
  ResourceID id = ResourceID::create(42, 7);
  EXPECT_TRUE(id.isValid());
  EXPECT_EQ(id.index, 42);
  EXPECT_EQ(id.generation, 7);
  
  // Test bool conversion
  EXPECT_TRUE(static_cast<bool>(id));
}

TEST(ResourceIDTest, Equality) {
  ResourceID id1 = ResourceID::create(42, 7);
  ResourceID id2 = ResourceID::create(42, 7);
  ResourceID id3 = ResourceID::create(42, 8); // Same index, different generation
  ResourceID id4 = ResourceID::create(43, 7); // Different index, same generation
  
  EXPECT_EQ(id1, id2);
  EXPECT_NE(id1, id3);
  EXPECT_NE(id1, id4);
  EXPECT_NE(id3, id4);
}

TEST(ResourceIDTest, InvalidEquality) {
  ResourceID id1 = ResourceID::invalid();
  ResourceID id2 = ResourceID::invalid();
  ResourceID id3 = ResourceID::create(42, 7);
  
  EXPECT_EQ(id1, id2);
  EXPECT_NE(id1, id3);
}

// Test string conversion
TEST(ResourceIDTest, ToString) {
  ResourceID id = ResourceID::create(42, 7);
  std::string str = id.toString();
  
  EXPECT_THAT(str, testing::HasSubstr("42"));
  EXPECT_THAT(str, testing::HasSubstr("7"));
  EXPECT_THAT(str, testing::HasSubstr("ResourceID"));
}

// Test compatibility with STL containers
TEST(ResourceIDTest, UsageInStdContainers) {
  // Create test IDs
  ResourceID id1 = ResourceID::create(1, 0);
  ResourceID id2 = ResourceID::create(2, 0);
  ResourceID id3 = ResourceID::create(3, 0);
  
  // Test with unordered_set
  std::unordered_set<ResourceID> idSet;
  EXPECT_TRUE(idSet.insert(id1).second);
  EXPECT_TRUE(idSet.insert(id2).second);
  EXPECT_TRUE(idSet.insert(id3).second);
  
  // Inserting a duplicate should fail
  EXPECT_FALSE(idSet.insert(id1).second);
  
  EXPECT_EQ(idSet.size(), 3);
  EXPECT_NE(idSet.find(id1), idSet.end());
  
  // Test with unordered_map
  std::unordered_map<ResourceID, int> idMap;
  idMap[id1] = 100;
  idMap[id2] = 200;
  idMap[id3] = 300;
  
  EXPECT_EQ(idMap.size(), 3);
  EXPECT_EQ(idMap[id1], 100);
  EXPECT_EQ(idMap[id2], 200);
  EXPECT_EQ(idMap[id3], 300);
}

// Test generation mechanism - simulating the ABA problem protection
TEST(ResourceIDTest, GenerationMechanism) {
  // Create a resource ID
  ResourceID id1 = ResourceID::create(42, 0);
  EXPECT_TRUE(id1.isValid());
  
  // Simulate resource deletion and reuse of the same index with a new generation
  ResourceID id2 = ResourceID::create(42, 1);
  EXPECT_TRUE(id2.isValid());
  
  // The old ID should be different from the new one
  EXPECT_NE(id1, id2);
  
  // Both IDs have the same index but different generations
  EXPECT_EQ(id1.index, id2.index);
  EXPECT_NE(id1.generation, id2.generation);
}

// Test hash function
TEST(ResourceIDTest, HashFunction) {
  // Create two different IDs
  ResourceID id1 = ResourceID::create(42, 7);
  ResourceID id2 = ResourceID::create(43, 7);
  
  // Create a copy of id1
  ResourceID id3 = ResourceID::create(42, 7);
  
  // Hash function should produce the same value for equal IDs
  std::hash<ResourceID> hasher;
  EXPECT_EQ(hasher(id1), hasher(id3));
  
  // Hash function should usually produce different values for different IDs
  // Note: This is probabilistic, collisions are possible but unlikely for simple cases
  EXPECT_NE(hasher(id1), hasher(id2));
}

// Edge case testing
TEST(ResourceIDTest, EdgeCases) {
  // Test with maximum valid index
  uint32_t maxIndex = std::numeric_limits<uint32_t>::max() - 1;
  ResourceID maxId = ResourceID::create(maxIndex, 0);
  EXPECT_TRUE(maxId.isValid());
  EXPECT_EQ(maxId.index, maxIndex);
  
  // Test with maximum generation
  uint32_t maxGen = std::numeric_limits<uint32_t>::max();
  ResourceID maxGenId = ResourceID::create(42, maxGen);
  EXPECT_TRUE(maxGenId.isValid());
  EXPECT_EQ(maxGenId.generation, maxGen);
  
  // ID with index = INVALID_INDEX should be invalid regardless of generation
  ResourceID edgeId = ResourceID::create(ResourceID::INVALID_INDEX, 42);
  EXPECT_FALSE(edgeId.isValid());
  EXPECT_EQ(edgeId.index, ResourceID::INVALID_INDEX);
}