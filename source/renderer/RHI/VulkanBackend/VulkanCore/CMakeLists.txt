file(GLOB_RECURSE sources CONFIGURE_DEPENDS *.cpp *.hpp)

add_library(VulkanCore STATIC)

target_sources(VulkanCore PUBLIC ${sources})

target_compile_options(VulkanCore PRIVATE 
  -Wno-missing-designated-field-initializers
)

message(STATUS "Vulkan DIR: " ${Vulkan_INCLUDE_DIR})
include_directories(${Vulkan_INCLUDE_DIR})

find_package(Vulkan REQUIRED)
find_package(volk REQUIRED)

target_link_libraries(VulkanCore PRIVATE volk::volk Vulkan::Vulkan GPUOpen::VulkanMemoryAllocator TracyClient)
