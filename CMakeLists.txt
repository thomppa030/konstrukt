cmake_minimum_required(VERSION 3.5)

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

######### Options ##########
option(KST_BUILD_TESTS "Build with Tests" OFF)
option(KST_USE_VULKAN "Build with Vulkan Support" ON)
option(KST_BUILD_RUNTIME "Build executable" ON)
######### Options End ######

if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)
find_package(fmt REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/source)

add_subdirectory(source/core)
add_subdirectory(source/renderer)

if(KST_BUILD_RUNTIME)
  add_subdirectory(konstrukt)
endif()

if(KST_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()


message(STATUS "Konstrukt Configuration:")
message(STATUS "  Vulkan support: ${KST_USE_VULKAN}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build Tests: ${KST_BUILD_TESTS}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
