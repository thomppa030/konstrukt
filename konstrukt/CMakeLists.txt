add_executable(konstrukt main.cc)

## target link konstrukt libraries here
find_package(glfw3 CONFIG REQUIRED)

target_link_libraries(konstrukt PRIVATE konstrukt_core konstrukt_rhi glfw)
