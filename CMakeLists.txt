cmake_minimum_required(VERSION 3.10)

project(CppTetris VERSION 1.0)
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
