cmake_minimum_required(VERSION 3.10)
project(MetroRouting LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(metro-routing
    src/main.cpp
    src/pathfinding.cpp
)

target_include_directories(metro-routing PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(metro-routing PRIVATE -O3 -march=native)
endif()
