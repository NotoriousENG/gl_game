# CMakeList.txt : CMake project for TinksWorld, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.12)

project ("game")

# C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# project includes
include_directories(include)

# set the library type based on the BUILD_SHARED_LIBS option
if(BUILD_SHARED_LIBS)
  set(LIB_TYPE SHARED)
else()
  set(LIB_TYPE STATIC)
endif()

add_library (${PROJECT_NAME} ${LIB_TYPE} "src/game.cpp")

target_include_directories(${PROJECT_NAME} PUBLIC ../engine/include)
target_link_libraries(${PROJECT_NAME} PUBLIC engine)

# set a variable for the directory where the assets are copied to, on non windows it is CMAKE_RUNTIME_OUTPUT_DIRECTORY, on windows it is CMAKE_RUNTIME_OUTPUT_DIRECTORY/(Debug|Release)
if(WIN32)
    set(FINAL_BINARY_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>)
else()
    set(FINAL_BINARY_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

add_custom_target(copy_assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/../assets ${FINAL_BINARY_DIR}/assets
    )
add_dependencies(${PROJECT_NAME} copy_assets)