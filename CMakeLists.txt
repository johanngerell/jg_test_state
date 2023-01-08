# CMake 3.15 fixed the /W4 vs /W3 bug with MSVC
cmake_minimum_required(VERSION 3.15)

project(jg_test_state)

set(CMAKE_CXX_STANDARD          14) 
set(CMAKE_CXX_STANDARD_REQUIRED ON) 
set(CMAKE_CXX_EXTENSIONS        OFF) 

include(CTest)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Werror -Wall -Wextra -Wpedantic -Wconversion)
elseif (MSVC)
    # warning 4668: 'foo' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'",
    # warning 4514: 'foo': unreferenced inline function has been removed
    # warning 4820: 'foo': 'N' bytes padding added after data member 'bar'
    # warning 4868: compiler may not enforce left-to-right evaluation order in braced initializer list
    # TODO: In debug /Zc:preprocessor /diagnostics:caret /RTCsu /sdl
    add_compile_options(/WX /W4 /Wall /wd4668 /wd4514 /wd4820 /wd4868)
endif ()

include_directories(${PROJECT_SOURCE_DIR}/inc)

add_library(jg_test_state INTERFACE inc/jg_test_state.h)

add_subdirectory(test)
