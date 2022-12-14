# CMake 3.15 fixed the /W4 vs /W3 bug with MSVC
cmake_minimum_required(VERSION 3.15)

project(jg_test_state)

set(CMAKE_CXX_STANDARD          14) 
set(CMAKE_CXX_STANDARD_REQUIRED ON) 
set(CMAKE_CXX_EXTENSIONS        OFF) 

include(CTest)

if (MSVC)
    # warning 4668: 'foo' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'",
    # warning 4514: 'foo': unreferenced inline function has been removed
    # TODO: In debug /Zc:preprocessor /diagnostics:caret /RTCsu /sdl
    add_compile_options(/WX /W4 /Wall /wd4668 /wd4514)
else ()
    add_compile_options(-Werror -Wall -Wextra -Wpedantic -Wconversion)
endif ()

include_directories(${PROJECT_SOURCE_DIR}/inc)

add_library(jg_test_state INTERFACE)

add_subdirectory(test)
