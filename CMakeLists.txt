cmake_minimum_required(VERSION 3.25)
project(OpenGLApp)
# --- C++ standard ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wno-sign-compare -static -mconsole -Werror=return-type")
# --- Include directories ---
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/headers
    ${CMAKE_SOURCE_DIR}/libs/glfw-3.4.bin.WIN64/include
    ${CMAKE_SOURCE_DIR}/libs/Assimp/include
)
# --- Collect all sources recursively ---
file(GLOB_RECURSE SOURCES_CPP ${CMAKE_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE SOURCES_C ${CMAKE_SOURCE_DIR}/src/*.c)
set(SOURCES ${SOURCES_CPP} ${SOURCES_C})
# --- Add executable ---
add_executable(${PROJECT_NAME} ${SOURCES})
# --- Output directory for executable (bin only) ---
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)
# --- Link GLFW (static) ---
target_compile_definitions(${PROJECT_NAME} PRIVATE GLFW_STATIC)
# --- Link libraries ---
target_link_libraries(${PROJECT_NAME}
    ${CMAKE_SOURCE_DIR}/libs/glfw-3.4.bin.WIN64/lib-mingw-w64/libglfw3.a
    ${CMAKE_SOURCE_DIR}/libs/Assimp/lib/libassimp.a
    # Link static zlib
    -static -lz
    opengl32
    user32
    gdi32
    shell32
    dbghelp
)