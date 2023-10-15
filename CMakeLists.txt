# CMakeList.txt : CMake project for TinksWorld, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.12)

project ("Application")

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# log if we are building shared or static libraries
message(STATUS "BUILD_SHARED_LIBS: ${BUILD_SHARED_LIBS}")

# set a define based on the BUILD_SHARED_LIBS option
if(BUILD_SHARED_LIBS)
  add_definitions(-DSHARED_GAME)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/testing) # simplify testing for shared libs
  set(GAME_LIBRARY_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/${CMAKE_SHARED_LIBRARY_PREFIX}game${CMAKE_SHARED_LIBRARY_SUFFIX}")
  add_compile_definitions(GAME_LIBRARY_PATH="${GAME_LIBRARY_PATH}")   # so we know where to hot reload from
endif()

set(GAME_NAME "GlGame" CACHE STRING "Name of the game")

# C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# log 5the build variant
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

# print the current cmake build environment
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

# add engine
add_subdirectory(engine)

# add game
add_subdirectory(game)

# Add source to this project's executable.
add_executable (${PROJECT_NAME} "src/main.cpp" "src/app.cpp")

# link game
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/game/include)
target_link_libraries(${PROJECT_NAME} PUBLIC game)

# project includes
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}/include")

# dependencies