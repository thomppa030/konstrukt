#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <core/memory/Allocator.hpp>

using namespace kst::core;
using ::testing::Return;
using ::testing::_;
using ::testing::Matcher;
using ::testing::MakeMatcher;
using ::testing::MatcherInterface;
using ::testing::MatchResultListener;

class MemoryAllocationMatcher : public MatcherInterface<const MemoryAllocation&> {
public:
    explicit MemoryAllocationMatcher(const MemoryAllocation& expected)
        : expected_(expected) {}

    bool MatchAndExplain(const MemoryAllocation& actual, MatchResultListener* listener) const override {
        bool matches = actual.mappedPtr == expected_.mappedPtr &&
                      actual.devicePtr == expected_.devicePtr &&
                      actual.size == expected_.size &&
                      actual.memoryType == expected_.memoryType;

        if (!matches && listener->IsInterested()) {
            *listener << "MemoryAllocation mismatch:\n"
                     << "  Expected: mappedPtr=" << expected_.mappedPtr
                     << ", devicePtr=" << expected_.devicePtr
                     << ", size=" << expected_.size
                     << ", memoryType=" << expected_.memoryType << "\n"
                     << "  Actual: mappedPtr=" << actual.mappedPtr
                     << ", devicePtr=" << actual.devicePtr
                     << ", size=" << actual.size
                     << ", memoryType=" << actual.memoryType;
        }

        return matches;
    }

    void DescribeTo(std::ostream* os) const override {
        *os << "MemoryAllocation matches:\n"
            << "  mappedPtr=" << expected_.mappedPtr << "\n"
            << "  devicePtr=" << expected_.devicePtr << "\n"
            << "  size=" << expected_.size << "\n"
            << "  memoryType=" << expected_.memoryType;
    }

private:
    const MemoryAllocation expected_;
};

inline Matcher<const MemoryAllocation&> MatchesMemoryAllocation(const MemoryAllocation& expected) {
    return MakeMatcher(new MemoryAllocationMatcher(expected));
}

class MockMemoryAllocator : public MemoryAllocator {
public:
    MOCK_METHOD(MemoryAllocation, allocate, (uint64_t size, MemoryDomain domain), (override));
    MOCK_METHOD(void, free, (const MemoryAllocation& allocation), (override));
    MOCK_METHOD(void*, map, (const MemoryAllocation& allocation), (override));
    MOCK_METHOD(void, unmap, (const MemoryAllocation& allocation), (override));
};

TEST(MemoryAllocatorTest, AllocateMemory) {
    MockMemoryAllocator allocator;
    
    MemoryAllocation expectedAllocation{
        .mappedPtr = nullptr,
        .devicePtr = 0x1000,
        .size = 1024,
        .memoryType = static_cast<uint64_t>(MemoryDomain::GPU_ONLY)
    };

    EXPECT_CALL(allocator, allocate(1024, MemoryDomain::GPU_ONLY))
        .WillOnce(Return(expectedAllocation));

    auto allocation = allocator.allocate(1024, MemoryDomain::GPU_ONLY);
    EXPECT_EQ(allocation.size, 1024);
    EXPECT_EQ(allocation.devicePtr, 0x1000);
    EXPECT_EQ(allocation.memoryType, static_cast<uint64_t>(MemoryDomain::GPU_ONLY));
}

TEST(MemoryAllocatorTest, MapMemory) {
    MockMemoryAllocator allocator;
    MemoryAllocation allocation{
        .mappedPtr = nullptr,
        .devicePtr = 0x2000,
        .size = 2048,
        .memoryType = static_cast<uint64_t>(MemoryDomain::CPU_AND_GPU)
    };

    void* expectedMappedPtr = reinterpret_cast<void*>(0x3000);
    EXPECT_CALL(allocator, map(MatchesMemoryAllocation(allocation)))
        .WillOnce(Return(expectedMappedPtr));

    void* mappedPtr = allocator.map(allocation);
    EXPECT_EQ(mappedPtr, expectedMappedPtr);
}

TEST(MemoryAllocatorTest, FreeMemory) {
    MockMemoryAllocator allocator;
    MemoryAllocation allocation{
        .mappedPtr = nullptr,
        .devicePtr = 0x4000,
        .size = 4096,
        .memoryType = static_cast<uint64_t>(MemoryDomain::GPU_ONLY)
    };

    EXPECT_CALL(allocator, free(MatchesMemoryAllocation(allocation)))
        .Times(1);

    allocator.free(allocation);
}

TEST(MemoryAllocatorTest, AllocationLifecycle) {
    MockMemoryAllocator allocator;
    
    MemoryAllocation expectedAllocation{
        .mappedPtr = nullptr,
        .devicePtr = 0x5000,
        .size = 8192,
        .memoryType = static_cast<uint64_t>(MemoryDomain::CPU_TO_GPU)
    };

    void* expectedMappedPtr = reinterpret_cast<void*>(0x6000);

    // Set up the expected sequence of calls
    {
        testing::InSequence seq;
        EXPECT_CALL(allocator, allocate(8192, MemoryDomain::CPU_TO_GPU))
            .WillOnce(Return(expectedAllocation));
        EXPECT_CALL(allocator, map(MatchesMemoryAllocation(expectedAllocation)))
            .WillOnce(Return(expectedMappedPtr));
        EXPECT_CALL(allocator, unmap(MatchesMemoryAllocation(expectedAllocation)));
        EXPECT_CALL(allocator, free(MatchesMemoryAllocation(expectedAllocation)));
    }

    // Perform the allocation lifecycle
    auto allocation = allocator.allocate(8192, MemoryDomain::CPU_TO_GPU);
    void* mappedPtr = allocator.map(allocation);
    EXPECT_EQ(mappedPtr, expectedMappedPtr);
    allocator.unmap(allocation);
    allocator.free(allocation);
}

TEST(MemoryAllocationTest, EqualityOperator) {
    // Create two identical allocations
    MemoryAllocation a1{
        .mappedPtr = nullptr,
        .devicePtr = 0x1000,
        .size = 1024,
        .memoryType = static_cast<uint64_t>(MemoryDomain::GPU_ONLY)
    };
    
    MemoryAllocation a2{
        .mappedPtr = nullptr,
        .devicePtr = 0x1000,
        .size = 1024,
        .memoryType = static_cast<uint64_t>(MemoryDomain::GPU_ONLY)
    };
    
    // Create a different allocation
    MemoryAllocation a3{
        .mappedPtr = nullptr,
        .devicePtr = 0x1000,
        .size = 2048,  // Different size
        .memoryType = static_cast<uint64_t>(MemoryDomain::GPU_ONLY)
    };
    
    MemoryAllocation a4{
        .mappedPtr = nullptr,
        .devicePtr = 0x1000,
        .size = 1024,
        .memoryType = static_cast<uint64_t>(MemoryDomain::CPU_AND_GPU)  // Different memory type
    };
    
    MemoryAllocation a5{
        .mappedPtr = nullptr,
        .devicePtr = 0x2000,  // Different device pointer
        .size = 1024,
        .memoryType = static_cast<uint64_t>(MemoryDomain::GPU_ONLY)
    };
    
    MemoryAllocation a6{
        .mappedPtr = reinterpret_cast<void*>(0x3000),  // Different mapped pointer
        .devicePtr = 0x1000,
        .size = 1024,
        .memoryType = static_cast<uint64_t>(MemoryDomain::GPU_ONLY)
    };
    
    // Test equality
    EXPECT_TRUE(a1 == a2);
    EXPECT_TRUE(a2 == a1);
    
    // Test inequality
    EXPECT_FALSE(a1 == a3);  // Different size
    EXPECT_FALSE(a1 == a4);  // Different memory type
    EXPECT_FALSE(a1 == a5);  // Different device pointer
    EXPECT_FALSE(a1 == a6);  // Different mapped pointer
}

TEST(MemoryAllocationTest, InequalityOperator) {
    // Create two identical allocations
    MemoryAllocation a1{
        .mappedPtr = nullptr,
        .devicePtr = 0x1000,
        .size = 1024,
        .memoryType = static_cast<uint64_t>(MemoryDomain::GPU_ONLY)
    };
    
    MemoryAllocation a2{
        .mappedPtr = nullptr,
        .devicePtr = 0x1000,
        .size = 1024,
        .memoryType = static_cast<uint64_t>(MemoryDomain::GPU_ONLY)
    };
    
    // Create a different allocation
    MemoryAllocation a3{
        .mappedPtr = nullptr,
        .devicePtr = 0x1000,
        .size = 2048,  // Different size
        .memoryType = static_cast<uint64_t>(MemoryDomain::GPU_ONLY)
    };
    
    // Test inequality operator
    EXPECT_FALSE(a1 != a2);
    EXPECT_TRUE(a1 != a3);
}

TEST(MemoryAllocationTest, Construction) {
    MemoryAllocation allocation{
        .mappedPtr = nullptr,
        .devicePtr = 0x1000,
        .size = 1024,
        .memoryType = static_cast<uint64_t>(MemoryDomain::CPU_TO_GPU)
    };
    
    EXPECT_EQ(allocation.mappedPtr, nullptr);
    EXPECT_EQ(allocation.devicePtr, 0x1000);
    EXPECT_EQ(allocation.size, 1024);
    EXPECT_EQ(allocation.memoryType, static_cast<uint64_t>(MemoryDomain::CPU_TO_GPU));
}
