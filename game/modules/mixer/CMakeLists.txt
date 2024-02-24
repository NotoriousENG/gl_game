# CMakeList.txt : CMake project for audio module
cmake_minimum_required (VERSION 3.12)

project ("mixer")

# C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# project includes
include_directories(include)

# add the library
add_library (${PROJECT_NAME} STATIC "src/mixer.cpp")

# dependencies
target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${SDL2_LIBRARIES})

target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_MIXER_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${SDL2_MIXER_LIBRARIES}) # ensure sdl2-mixer "extensions" are installed for .ogg: https://www.reddit.com/r/cataclysmdda/comments/glxgtb/fix_for_sound_problem_when_compiling_in_windows/
