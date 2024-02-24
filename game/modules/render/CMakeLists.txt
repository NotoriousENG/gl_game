# CMakeList.txt : CMake project for input module
cmake_minimum_required (VERSION 3.12)

project ("render")

# C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# project includes
include_directories(include)

# add the library
add_library (${PROJECT_NAME} STATIC "src/renderer.cpp" 
"src/window.cpp" "src/shader.cpp" "src/texture.cpp" 
"src/sprite-batch.cpp"
"src/spritesheet.cpp" "src/font.cpp")

# dependencies

target_include_directories(${PROJECT_NAME} PUBLIC ${GLAD_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC glad)

target_include_directories(${PROJECT_NAME} PUBLIC ${GLM_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PUBLIC ${FREETYPE_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC ${FREETYPE_INCLUDE_DIRS})

target_include_directories(${PROJECT_NAME} PUBLIC ${JSON_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC nlohmann_json)

target_include_directories(${PROJECT_NAME} PUBLIC ${TMX_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC tmxlite)

target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${SDL2_LIBRARIES})