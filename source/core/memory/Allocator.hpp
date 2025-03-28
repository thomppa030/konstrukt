#pragma once

#include <cstdint>

#include <core/CoreTypes.hpp>

namespace kst::core {

  struct MemoryAllocation {
    void* mappedPtr;
    uint64_t devicePtr;
    uint64_t size;
    uint64_t memoryType;
  };

  inline auto operator==(const MemoryAllocation& lhs, const MemoryAllocation& rhs) -> bool {
    return lhs.mappedPtr == rhs.mappedPtr && lhs.devicePtr == rhs.devicePtr &&
           lhs.size == rhs.size && lhs.memoryType == rhs.memoryType;
  }

  inline auto operator!=(const MemoryAllocation& lhs, const MemoryAllocation& rhs) -> bool {
    return !(lhs == rhs);
  }

  class MemoryAllocator {
  public:
    virtual ~MemoryAllocator() = default;

    virtual auto allocate(uint64_t size, MemoryDomain domain) -> MemoryAllocation = 0;

    virtual void free(const MemoryAllocation& allocation) = 0;

    virtual auto map(const MemoryAllocation& allocation) -> void* = 0;

    virtual void unmap(const MemoryAllocation& allocation) = 0;
  };
} // namespace kst::core
