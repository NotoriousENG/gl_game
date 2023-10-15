cmake_minimum_required (VERSION 3.8)

project ("glad" C)
set(CMAKE_BUILD_TYPE Release)

include_directories(include)
add_library("glad" src/glad.c)
