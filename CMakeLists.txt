cmake_minimum_required(VERSION 3.20)

project(MetalRenderer LANGUAGES CXX OBJCXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_library(METAL_FRAMEWORK Metal REQUIRED)
find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
find_library(QUARTZCORE_FRAMEWORK QuartzCore REQUIRED)
find_library(APPKIT_FRAMEWORK AppKit REQUIRED)
find_library(METALKIT_FRAMEWORK MetalKit REQUIRED)

add_executable(MetalRenderer
src/main.mm
src/AppDelegate.mm
src/Renderer.cpp
src/MetalImplementation.cpp)

target_include_directories(MetalRenderer PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/metal-cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/metal-cpp-extensions"
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(MetalRenderer PRIVATE
    ${METAL_FRAMEWORK}
    ${FOUNDATION_FRAMEWORK}
    ${QUARTZCORE_FRAMEWORK}
    ${APPKIT_FRAMEWORK}
    ${METALKIT_FRAMEWORK}
    "-framework CoreGraphics"
    "-framework IOKit"
    "-framework CoreFoundation"
)
