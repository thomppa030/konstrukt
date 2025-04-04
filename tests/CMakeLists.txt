find_package(GTest REQUIRED)

# Create test executable
add_executable(konstrukt_tests
    main.cpp
    core/memory/AllocatorTest.cpp
    core/CoreTypesTest.cpp
    core/config/ConfigTest.cpp
    renderer/core/GraphicsTypeTest.cpp
    core/log/LoggerTest.cpp
    renderer/core/GraphicsHandlesTest.cpp
    renderer/resources/ResourceIDTest.cpp
)

# Link against GTest, GMock and our libraries
target_link_libraries(konstrukt_tests PRIVATE
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    GTest::gmock_main
    konstrukt_core
    konstrukt_renderer
)

# Add test to CTest
add_test(NAME konstrukt_tests COMMAND konstrukt_tests) 