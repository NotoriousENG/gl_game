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

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

# add engine
add_subdirectory(engine)

# add game
add_subdirectory(game)

# Add source to this project's executable.
add_executable (${PROJECT_NAME} "src/main.cpp" "src/app.cpp")

# link engine
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/engine/include)
target_link_libraries(${PROJECT_NAME} PUBLIC engine)

# link game
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/game/include)
target_link_libraries(${PROJECT_NAME} PUBLIC game)

# project includes
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}/include")

if (WIN32)
    # /ENTRY:mainCRTStartup keeps the same "main" function instead of requiring "WinMain"
    # if release
    if (CMAKE_BUILD_TYPE MATCHES Release)
        set(SUBSYSTEM_LINKER_OPTIONS "/SUBSYSTEM:WINDOWS" "/ENTRY:mainCRTStartup" "/LTCG")
    endif()
    target_link_options(${PROJECT_NAME} PRIVATE ${SUBSYSTEM_LINKER_OPTIONS})

    # windows cmake puts into the build type dir
    add_custom_target(copy_assets
        # log Copying on Windows to 
        COMMAND echo Copying assets to ${CMAKE_CURRENT_BINARY_DIR}/assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets
    )
    add_dependencies(${PROJECT_NAME} copy_assets) 
else()
    add_custom_target(copy_assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets
    )
    add_dependencies(${PROJECT_NAME} copy_assets)
endif()

if( ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    add_custom_target(copy_html
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/web/index.html ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.html
        # replace "index.js" in ${PROJECT_NAME}.html with "${PROJECT_NAME}.js"
        COMMAND sed -i "s/index.js/${PROJECT_NAME}.js/g" ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.html
    )
    add_dependencies(${PROJECT_NAME} copy_html)
endif()