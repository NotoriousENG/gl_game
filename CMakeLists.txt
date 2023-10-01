# CMakeList.txt : CMake project for TinksWorld, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("GlGame")

set(GAME_NAME "GlGame" CACHE STRING "Name of the game")
option(ENABLE_DEBUG_COLLISIONS "Enable DEBUG_COLLISIONS" OFF)

# C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(NO_MEDIA ON CACHE BOOL "Disable media support in libdatachannel")
SET(USE_SYSTEM_JSON ON CACHE BOOL "Use system Nlohmann JSON")
SET(NO_EXAMPLES ON CACHE BOOL "Disable examples")

message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
# log 5the build variant
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

# print the current cmake build environment
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

if( ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    add_compile_definitions("EMSCRIPTEN")
    set(USE_FLAGS "-s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_MIXER=2 -s USE_SDL_TTF=2 -s USE_WEBGL2=1 -s MIN_WEBGL_VERSION=2 -s FULL_ES3 -s ASSERTIONS=2 -s INITIAL_MEMORY=64MB -s TOTAL_STACK=32MB -s ALLOW_MEMORY_GROWTH=1 -s NO_DISABLE_EXCEPTION_CATCHING -gsource-map --preload-file assets --use-preload-plugins")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${USE_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${USE_FLAGS}")
    set(CMAKE_EXECUTABLE_SUFFIX .html)
else()
    # if CLANG/ GCC use GNU99 (This ensures that aflecs ddons that rely on time & socket functions are compiled correctly.)
    if (CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID MATCHES "GNU")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
    endif()
    find_package(SDL2 REQUIRED)
    find_package(sdl2-mixer REQUIRED)
    find_package(sdl2-image REQUIRED)
    find_package(sdl2-ttf REQUIRED)
endif()

# Add library subdirectories
add_subdirectory(lib/glad)
add_subdirectory(lib/flecs)

# Set the TMXLITE_STATIC_LIB option to TRUE to request static linking
SET(TMXLITE_STATIC_LIB TRUE CACHE BOOL "Should tmxlite be built as a static or shared lib?")
add_subdirectory(lib/tmxlite/tmxlite)

add_subdirectory(lib/json)

# Add source to this project's executable.
add_executable (${PROJECT_NAME} "src/main.cpp" "src/window.cpp" "src/renderer.cpp" 
    "src/texture.cpp" "src/shader.cpp" "src/sprite-batch.cpp" "src/input.cpp"
    "src/tilemap.cpp"
)

if( ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    add_subdirectory(lib/datachannel-wasm)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/lib/libdatachannel-wasm/wasm/include)
    target_link_libraries(${PROJECT_NAME} PRIVATE datachannel-wasm)
else()
    add_subdirectory(lib/libdatachannel)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/lib/libdatachannel/include)
    target_link_libraries(${PROJECT_NAME} PRIVATE datachannel-static)
endif()


# self published libraries
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/lib/glad/include)
target_link_libraries(${PROJECT_NAME} PRIVATE glad)

target_link_libraries(${PROJECT_NAME} PRIVATE flecs::flecs_static)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/lib/tmxlite/tmxlite/include)
target_link_libraries(${PROJECT_NAME} PRIVATE tmxlite)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/lib/glm)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/lib/json/include)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json)

# if this is a release build #define DEBUG 1
if (NOT CMAKE_BUILD_TYPE MATCHES Release) # append AND false to disable in debug
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG=1)
endif()


if(ENABLE_DEBUG_COLLISIONS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG_COLLISIONS=1)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE GAME_NAME="${GAME_NAME}")

# project includes
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}/include")

# dependencies
target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES})

target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_IMAGE_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_MIXER_LIBRARIES}) # ensure sdl2-mixer "extensions" are installed for .ogg: https://www.reddit.com/r/cataclysmdda/comments/glxgtb/fix_for_sound_problem_when_compiling_in_windows/
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_TTF_LIBRARIES})

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