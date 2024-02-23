# CMakeList.txt : CMake project for input module
cmake_minimum_required (VERSION 3.12)

project ("input")

# C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    # if CLANG/ GCC use GNU99 (This ensures that aflecs ddons that rely on time & socket functions are compiled correctly.)
    if (CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID MATCHES "GNU")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
    endif()
endif()

# project includes
include_directories(include)

# add the library
add_library (${PROJECT_NAME} STATIC "src/input.cpp")

target_include_directories(${PROJECT_NAME} PUBLIC ${GLM_INCLUDE_DIRS})

# dependencies
target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${SDL2_LIBRARIES})