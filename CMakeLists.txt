cmake_minimum_required (VERSION 3.8)

project(ylang LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(core)
#add_subdirectory(vm)
add_subdirectory(tests/devtest)

if (MSVC)
    add_compile_options(/utf-8)
endif()
