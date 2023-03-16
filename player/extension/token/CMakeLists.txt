cmake_minimum_required(VERSION 2.6)

# Locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

file(GLOB TEST_SOURCES "./*.c")

include_directories ("./")

set (CMAKE_C_FLAGS "-std=gnu99 ${CMAKE_C_FLAGS}")

# Link runTests with what we want to test and the GTest and pthread library
add_executable(runTests test_token.cpp ${TEST_SOURCES})
target_link_libraries(runTests ${GTEST_LIBRARIES} pthread)
