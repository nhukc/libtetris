cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

project(LibTetris VERSION 1.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
configure_file(include/TetrisConfig.hpp.in include/TetrisConfig.hpp)

include_directories("include")

add_subdirectory(src)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(test)
endif()

set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra")

add_executable(test1 main.cpp)
target_link_libraries(test1 libtetris bot)
