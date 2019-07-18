cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 14)

project(CSL)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

include_directories($(.))

file(GLOB CORE "*.h" "main.cpp" )
source_group("Src" FILES ${CORE})

file(GLOB SHADERS "shader_suite/*.h" "shader_suite/*.cpp")
source_group("Src\\shader_suite" FILES ${SHADERS})

add_executable(CSL_test ${CORE} ${SHADERS} )