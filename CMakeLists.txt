cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 14)

project(CSL)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

include_directories($())

file(GLOB headers "*.h" "main.cpp" )

add_executable(CLS_test ${headers} )