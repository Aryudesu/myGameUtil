cmake_minimum_required(VERSION 3.16)
project(myGameUtil LANGUAGES CXX)

add_library(myGameUtil INTERFACE)
target_include_directories(myGameUtil INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_compile_features(myGameUtil INTERFACE cxx_std_17)
