cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 14)

project(CSL_ShaderSuite)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

include_directories($(.) $("../") )

file(GLOB ShaderSuiteApp "*.h" "*.cpp" "main.cpp" )
source_group("Src" FILES ${ShaderSuiteApp})

file(GLOB Shaders "shaders/*.h" "shaders/*.cpp")
source_group("Src\\Shaders" FILES ${Shaders})

file(GLOB CSL_CORE "./../csl/*.hpp")
source_group("Src\\csl" FILES ${CSL_CORE})


add_executable(CSL_ShaderSuite ${ShaderSuiteApp} ${Shaders} ${CSL_CORE} )