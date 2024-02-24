# CMakeList.txt : CMake project for input module
cmake_minimum_required (VERSION 3.12)

project ("input")

# C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# project includes
include_directories(include)

# add the library
add_library (${PROJECT_NAME} STATIC "src/input.cpp")

target_include_directories(${PROJECT_NAME} PUBLIC ${GLM_INCLUDE_DIRS})

# dependencies
target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${SDL2_LIBRARIES})