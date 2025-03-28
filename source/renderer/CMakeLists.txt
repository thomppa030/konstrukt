add_library(konstrukt_renderer STATIC)

target_include_directories(konstrukt_renderer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
)

target_sources(konstrukt_renderer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/core/GraphicsType.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/GraphicsHandles.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/GraphicsContext.hpp
)

# Link against spdlog
target_link_libraries(konstrukt_renderer PRIVATE konstrukt_core)
