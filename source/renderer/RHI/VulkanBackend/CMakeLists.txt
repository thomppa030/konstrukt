add_library(konstrukt_vulkan STATIC)

find_package(volk REQUIRED)

target_sources(konstrukt_vulkan PRIVATE
  VulkanContext.hpp
  VulkanContext.cc
)

target_link_libraries(konstrukt_vulkan PRIVATE GPUOpen::VulkanMemoryAllocator volk::volk konstrukt_rhi VulkanCore)
