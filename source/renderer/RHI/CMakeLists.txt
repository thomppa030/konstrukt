add_library(konstrukt_rhi STATIC)

target_include_directories(konstrukt_rhi PUBLIC
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/source
)

add_subdirectory(VulkanBackend)
add_subdirectory(VulkanBackend/VulkanCore)

target_sources(konstrukt_rhi PUBLIC
  GraphicsContext.hpp
  GraphicsContext.cc
)


target_link_libraries(konstrukt_rhi PRIVATE konstrukt_core konstrukt_vulkan)
