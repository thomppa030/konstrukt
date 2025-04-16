cmake_minimum_required(VERSION 3.30)

project(
  konstrukt
  VERSION 0.1.0
  DESCRIPTION "Data Oriented Game Engine"
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS_DIRECTORY "${CMAKE_SOURCE_DIR}")

option(KST_BUILD_RUNTIME "Build executable" ON)

##### Unit Testing Flags #####
option(KST_BUILD_TESTS "Build with Tests" OFF)
option(KST_BUILD_COVERAGE "Build with Coverage reporting" OFF)
##### Unit Testing Flags END##

list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}/generators")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/generators")

if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()


add_subdirectory(vendor)

include_directories(
  $ {CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include
)

add_subdirectory(source)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/source")

if(KST_BUILD_RUNTIME)
add_subdirectory(konstrukt)
endif()

if(KST_BUILD_TESTS)
enable_testing()
add_subdirectory(tests)
endif()

message(STATUS "Konstrukt Configuration")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build Tests: ${KST_BUILD_TESTS}")
message(STATUS "  Build Coverage: ${KST_BUILD_COVERAGE}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
