# CMakeList.txt : CMake project for TinksWorld, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.12)

project ("GlGame")

if( ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    add_compile_definitions("EMSCRIPTEN")
    
    set(BUILD_SHARED_LIBS OFF)

    set(ENV{EMCC_DEBUG} 1)
    set(USE_FLAGS "-s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_MIXER=2 -s USE_SDL_TTF=2 -s USE_WEBGL2=1 -s MIN_WEBGL_VERSION=2 -s FULL_ES3 -s INITIAL_MEMORY=64MB -s TOTAL_STACK=32MB -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=1 -s NO_DISABLE_EXCEPTION_CATCHING -g3 --preload-file assets --use-preload-plugins")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${USE_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${USE_FLAGS}")
    set(CMAKE_EXECUTABLE_SUFFIX .html)
else()
    option(BUILD_SHARED_LIBS "Build shared libraries" ON) # when distributing, set to OFF
endif()

# log if we are building shared or static libraries
message(STATUS "BUILD_SHARED_LIBS: ${BUILD_SHARED_LIBS}")

if (WIN32)
    # vs2022 and vscode CMake extension put output files in different places do this for now
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME})
endif()

# set a define based on the BUILD_SHARED_LIBS option
if(BUILD_SHARED_LIBS)
  add_compile_definitions(SHARED_GAME=1)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}) # simplify testing for shared libs
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
endif()

if( ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    add_custom_target(copy_html
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/web/index.html ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.html
        # replace "index.js" in ${PROJECT_NAME}.html with "${PROJECT_NAME}.js"
        COMMAND sed -i "s/index.js/${PROJECT_NAME}.js/g" ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.html
    )
    add_dependencies(${PROJECT_NAME} copy_html)
endif()