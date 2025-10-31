cmake_minimum_required (VERSION 3.31)

project(ylang LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options(/utf-8)
endif()

add_subdirectory(core)
add_subdirectory(tests/devtest)
add_subdirectory(tests/ylang)
