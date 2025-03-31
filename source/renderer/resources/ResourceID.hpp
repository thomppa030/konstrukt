#pragma once

#include <cstdint>
#include <format>
#include <functional>
#include <limits>
#include <string>

namespace kst::renderer::resource {

  /**
   * @brief Resource identifier with generation counting to avoid the ABA problem
   *
   * ResourceID is a handle to a resource that combines an index with a generation counter.
   * This design prevents the "ABA problem" where a resource is deleted and a new one
   * is created at the same index, which could lead to dangling references.
   *
   * The generation is incremented each time a resource slot is reused, ensuring
   * that old handles to a slot that has been recycled will be detected as invalid.
   */
  struct ResourceID {
    // Constants for easier configuration
    static constexpr uint32_t INVALID_INDEX = std::numeric_limits<uint32_t>::max();

    // Member variables
    uint32_t index      = INVALID_INDEX; // Default to invalid, must be set to a valid value
    uint32_t generation = 0;             // Incremented each time an index is recycled

    /**
     * @brief Create an invalid resource ID
     * @return ResourceID An invalid resource identifier
     */
    static constexpr auto invalid() -> ResourceID {
      return ResourceID{.index = INVALID_INDEX, .generation = 0};
    }

    /**
     * @brief Create a resource ID with specific index and generation
     * @param idx The index to use
     * @param gen The generation to use
     * @return ResourceID The constructed resource identifier
     */
    static constexpr auto create(uint32_t idx, uint32_t gen = 0) -> ResourceID {
      return ResourceID{.index = idx, .generation = gen};
    }

    /**
     * @brief Check if this resource ID is valid
     * @return bool True if the resource ID is valid, false otherwise
     */
    [[nodiscard]] constexpr auto isValid() const -> bool { return index != INVALID_INDEX; }

    /**
     * @brief Equality comparison operator
     * @param other The resource ID to compare with
     * @return bool True if both IDs are equal (same index and generation)
     */
    constexpr auto operator==(const ResourceID& other) const -> bool {
      return index == other.index && generation == other.generation;
    }

    /**
     * @brief Inequality comparison operator
     * @param other The resource ID to compare with
     * @return bool True if the IDs are not equal
     */
    constexpr auto operator!=(const ResourceID& other) const -> bool { return !(*this == other); }

    /**
     * @brief Boolean conversion operator
     * @return bool True if the resource ID is valid
     */
    explicit constexpr operator bool() const { return isValid(); }

    /**
     * @brief Convert to string representation
     * @return std::string String representation of the resource ID
     */
    auto toString() const -> std::string {
      return std::format("ResourceID(index={}, generation={})", index, generation);
    }
  };

} // namespace kst::renderer::resource

namespace std {
  /**
   * @brief Hash function specialization for ResourceID
   *
   * This allows ResourceID to be used as a key in unordered containers.
   */
  template <>
  struct hash<kst::renderer::resource::ResourceID> {
    auto operator()(const kst::renderer::resource::ResourceID& rID) const noexcept -> std::size_t {
      // Combine hashes of index and generation using FNV-1a inspired approach
      std::size_t hash1 = std::hash<uint32_t>{}(rID.index);
      std::size_t hash2 = std::hash<uint32_t>{}(rID.generation);

      return hash1 ^ (hash2 + 0x9e3779b9 + (hash1 << 6U) + (hash1 >> 2U));
    }
  };
} // namespace std
