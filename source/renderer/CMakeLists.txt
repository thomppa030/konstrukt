add_library(konstrukt_renderer STATIC)

find_package(Boost REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

target_include_directories(konstrukt_renderer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
)

# Core renderer files
target_sources(konstrukt_renderer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/core/GraphicsType.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/GraphicsHandles.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/GraphicsContext.hpp
)

# Command system
target_sources(konstrukt_renderer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/commands/RenderCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/commands/RenderCommandBuffer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/commands/RenderCommandBuffer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/commands/CommandTranslator.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/commands/CommandTranslator.cc
)

# Framegraph system
target_sources(konstrukt_renderer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/framegraph/Framegraph.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/framegraph/Framegraph.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/framegraph/FramegraphBuilder.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/framegraph/FramegraphBuilder.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/framegraph/RenderPass.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/framegraph/RenderPass.cc
)

# Resources system
target_sources(konstrukt_renderer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/ResourceID.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/ResourceRegistry.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/ResourceRegistry.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/ResourceManager.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/ResourceManager.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/RenderResource.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/RenderResource.cc
)

# Main renderer
target_sources(konstrukt_renderer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer.cc
)

# Vulkan implementation
target_sources(konstrukt_renderer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/core/vulkan/VulkanContext.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/vulkan/VulkanContext.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/core/vulkan/VulkanCommandRecorder.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/vulkan/VulkanCommandRecorder.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/core/vulkan/VulkanDevice.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/vulkan/VulkanDevice.cc
)

# Link libraries
target_link_libraries(konstrukt_renderer PRIVATE 
    Boost::boost 
    glm::glm
    Vulkan::Vulkan
    glfw
    konstrukt_core
)
