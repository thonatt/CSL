cmake_minimum_required(VERSION 3.11)

project(CSL_ShaderSuite)

#set(CMAKE_CXX_STANDARD 17)
#message("Your C++ compiler supports these C++ features:")
#foreach(i ${CMAKE_CXX_COMPILE_FEATURES})
#  message("${i}")
#endforeach()

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

if (MSVC)
	add_definitions(/bigobj /Zc:__cplusplus /std:c++17 /Zc:preprocessor)
endif(MSVC)

# CSL
file(GLOB ShaderSuiteApp "*.hpp" "*.cpp" "main.cpp" )
source_group("Src" FILES ${ShaderSuiteApp})

file(GLOB Shaders "shaders/*.h" "shaders/*.cpp")
source_group("Src\\Shaders" FILES ${Shaders})

set(CSL_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/../include")

file(GLOB CSL_FILES "${CSL_INCLUDE}/csl/*.hpp")
source_group("Src\\csl" FILES ${CSL_FILES})

file(GLOB CSL_GLSL "${CSL_INCLUDE}/csl/glsl/*.hpp")
source_group("Src\\glsl" FILES ${CSL_GLSL})

file(GLOB CSL_IMGUI "${CSL_INCLUDE}/csl/imgui/*.hpp")
source_group("Src\\imgui" FILES ${CSL_IMGUI})

set(CSL_SHADER_SUITE_SHADERS_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/shaders/")

set(CSL_SHADER_SUITE_RESOURCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../resources/")
add_compile_definitions(CSL_SHADER_SUITE_RESOURCE_PATH=\"${CSL_SHADER_SUITE_RESOURCE_PATH}\")

set(PICOGL_USE_FRAMEWORK ON)
include(FetchContent)
FetchContent_Declare(
	picogl
	GIT_REPOSITORY "https://github.com/thonatt/picogl"
	GIT_TAG "0c6dd1436b651159f9c188b8d28539ad511c392a"
)
FetchContent_MakeAvailable(picogl)

add_executable(CSL_ShaderSuite 
	${ShaderSuiteApp} ${Shaders} ${CSL_FILES} ${CSL_GLSL} ${CSL_IMGUI}
)

target_include_directories(CSL_ShaderSuite PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR} ${CSL_INCLUDE} ${CSL_SHADER_SUITE_SHADERS_INCLUDE}
)

target_link_libraries(CSL_ShaderSuite PUBLIC
	picogl::framework
)
