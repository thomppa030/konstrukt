add_library(konstrukt_core STATIC)

target_include_directories(konstrukt_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/vendor/fmt/include
    ${CMAKE_SOURCE_DIR}/vendor/spdlog/include
    ${CMAKE_SOURCE_DIR}/vendor/vulkan-headers/include
    ${CMAKE_SOURCE_DIR}/vendor/nlohmann_json/include
)

target_sources(konstrukt_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/memory/Allocator.hpp
)

# Add the logger files
target_sources(konstrukt_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/log/Logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/log/Logger.hpp
)

# Add the application files
target_sources(konstrukt_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/application/Application.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/application/Application.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/application/ApplicationWindow.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/application/ApplicationWindow.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/application/Layer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/application/Layer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/application/LayerStack.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/application/LayerStack.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/application/VulkanTestLayer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/application/VulkanTestLayer.hpp
)

# Add the config files
target_sources(konstrukt_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/config/Config.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/config/Config.cc
)

# Add the utility files
target_sources(konstrukt_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/FileUtils.hpp
)
# Find required packages
find_package(Boost CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

# Link against libraries
target_link_libraries(konstrukt_core PRIVATE 
    boost::boost
    spdlog::spdlog_header_only
    glfw
    nlohmann_json::nlohmann_json
)
