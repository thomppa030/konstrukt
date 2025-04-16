add_library(konstrukt_app STATIC)

target_include_directories(konstrukt_app PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(konstrukt_app PUBLIC
  Layer.hpp
  LayerStack.hpp
  LayerStack.cc
)

target_link_libraries(konstrukt_app PRIVATE konstrukt_core)
