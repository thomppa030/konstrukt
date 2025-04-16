add_library(konstrukt_core STATIC)

target_include_directories(konstrukt_core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)


target_sources(konstrukt_core PRIVATE
  Logger.hpp
  Logger.cc
)

target_link_libraries(konstrukt_core PRIVATE
  spdlog::spdlog_header_only
)

