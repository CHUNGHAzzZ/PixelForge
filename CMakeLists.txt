cmake_minimum_required(VERSION 3.21)

project(PixelForge
    VERSION 0.1.0
    LANGUAGES CXX
)

option(PIXELFORGE_BUILD_TESTS "Build PixelForge tests" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(src)

if(PIXELFORGE_BUILD_TESTS)
    add_subdirectory(test)
endif()
