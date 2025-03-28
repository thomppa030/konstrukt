add_library(konstrukt_core STATIC)

target_include_directories(konstrukt_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(konstrukt_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/memory/Allocator.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GraphicsType.hpp
)

# Add the new logger files
target_sources(konstrukt_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/log/Logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/log/Logger.hpp
)

# Find spdlog package
find_package(spdlog REQUIRED)

# Link against spdlog
target_link_libraries(konstrukt_core PRIVATE spdlog::spdlog)
