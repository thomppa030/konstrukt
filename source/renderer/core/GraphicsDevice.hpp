#pragma once

#include <cstdint>
#include <string>

#include "CoreTypes.hpp"
#include "GraphicsType.hpp"

namespace kst::renderer::core {

  /*
   *  The GraphicsDevice represents the physical graphics hardware and its capabilities.
   *
   *  Provides information about the supported features, limits, and memory properties
   */
  class GraphicsDevice {
  public:
    virtual ~GraphicsDevice() = default;

    /*
     *  Check if specific feature is supported by the device.
     *
     *  @param feature to check for support
     *  @return True if the feature is supported
     */
    virtual auto supportsFeature(kst::core::FeatureFlag feature) -> bool = 0;

    /*
     *  Get the maximum texture dimensions supported by the device.
     *
     *  @return Maximum texture size in pixels (width/height)
     */
    virtual auto getMaxTextureSize() const -> bool = 0;

    /*
     *  Get the maximum number of compute work groups supported
     *
     *  @param maxX Output Parameter for maximum work groups in X dimension
     *  @param maxY Output Parameter fot maximum work groups in Y dimension
     *  @param maxZ Output Parameter for maximum work groups in Z dimension
     */
    virtual void getMaxComputeWorkGroups(uint32_t& maxX, uint32_t& maxY, uint32_t& maxZ) = 0;

    /*
     *  Get the limits of the device
     *
     *  @return Structure containing memory properties
     */
    virtual auto getDeviceLimits() const -> kst::core::Limits = 0;

    /*
     *  Get the memory properties of the device.
     *
     *  @return Structure containing memory properties
     */
    virtual auto getMemoryProperties() const -> kst::core::MemoryProperties = 0;

    /*
     *  Get the name of the device
     *
     *  @return the Device name as a string
     */
    virtual auto getDeviceName() const -> std::string = 0;

    /*
     *  Get the vendor of the device
     *
     *  @return Device vendor as a string
     */
    virtual auto getDeviceVendor() const -> std::string = 0;

    /*
     *  Get the API Version supported by the device
     *
     *  @param major Major version
     *  @param minor Minor version
     *  @param patch Patch version
     */
    virtual void getAPIVersion(uint32_t& major, uint32_t& minor, uint32_t& patch) = 0;

    /*
     *  Get the device type (discrete GPU, integrated GPU, software, etc.)
     *  @return Device type
     */
    virtual auto getDeviceType() const -> ::kst::core::DeviceType = 0;

    /*
     *  Get the available memory on the device.
     *
     *  @return Available memory in bytes
     */
    virtual auto getAvailableMemory() -> uint64_t = 0;
  };
} // namespace kst::renderer::core
