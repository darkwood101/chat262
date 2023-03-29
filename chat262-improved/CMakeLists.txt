cmake_minimum_required(VERSION 3.13)

project(
    chat262
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src)

option(TESTING "Build Chat 262 tests" OFF)
if (TESTING)
    add_subdirectory(tests)
endif()
