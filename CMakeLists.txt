cmake_minimum_required(VERSION 3.25)

set(app MetalBareBones)

project(${app} LANGUAGES CXX OBJCXX OBJC)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_OBJCXX_STANDARD 23)

execute_process(
    COMMAND
    ${CMAKE_SOURCE_DIR}/setupMetalCpp.sh ${CMAKE_BINARY_DIR}
)

include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

add_executable(${app} MetalSetup.cpp Renderer.cpp ShaderLoader.cpp main.mm)

include_directories(${CMAKE_BINARY_DIR}/_deps/metal-cpp)

file(GLOB shaderSources "*.metal" ShaderTypes.h)
add_custom_target(
    shaderCopy ALL
    ${CMAKE_COMMAND} -E copy ${shaderSources} ${CMAKE_CURRENT_BINARY_DIR}/
)
add_dependencies(${app} shaderCopy)

target_link_libraries(
    ${app}
    glfw
    "-framework Metal"
    "-framework QuartzCore"
    "-framework CoreFoundation"
    "-framework AppKit"
    "-framework Cocoa"
)