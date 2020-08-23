cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)

project(CSL_ShaderSuite)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

if (MSVC)
	add_definitions(/bigobj)
endif(MSVC)

# CSL
file(GLOB ShaderSuiteApp "*.hpp" "*.cpp" "main.cpp" )
source_group("Src" FILES ${ShaderSuiteApp})

file(GLOB Shaders "shaders/*.h" "shaders/*.cpp")
source_group("Src\\Shaders" FILES ${Shaders})

file(GLOB CSL_CORE "./../csl/*.hpp")
source_group("Src\\csl" FILES ${CSL_CORE})

file(GLOB CSL_CORE_v2 "./../v2/*.hpp")
source_group("Src\\csl_v2" FILES ${CSL_CORE_v2})

file(GLOB CSL_GLSL "./../v2/glsl/*.hpp")
source_group("Src\\glsl" FILES ${CSL_GLSL})

set(CSL_CORE_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/../")
set(CSL_SHADER_SUITE_SHADERS_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/shaders/")

# EXTLIBS
set(CSL_SHADER_SUITE_EXTLIBS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/extlibs/")

### GLAD
set(CSL_SHADER_SUITE_GLAD_INCLUDE "${CSL_SHADER_SUITE_EXTLIBS_PATH}/glad/include/")
set(CSL_SHADER_SUITE_GLAD_SRC "${CSL_SHADER_SUITE_EXTLIBS_PATH}/glad/src/glad.c")

### IMGUI
set(CSL_SHADER_SUITE_IMGUI_PATH "${CSL_SHADER_SUITE_EXTLIBS_PATH}/imgui/")
file(GLOB CSL_SHADER_SUITE_IMGUI_HEADERS "${CSL_SHADER_SUITE_IMGUI_PATH}/*.h"
	"${CSL_SHADER_SUITE_IMGUI_PATH}/examples/imgui_impl_glfw.h"
	"${CSL_SHADER_SUITE_IMGUI_PATH}/examples/imgui_impl_opengl3.h")
file(GLOB CSL_SHADER_SUITE_IMGUI_SRC "${CSL_SHADER_SUITE_IMGUI_PATH}/*.cpp"
	"${CSL_SHADER_SUITE_IMGUI_PATH}/examples/imgui_impl_glfw.cpp"
	"${CSL_SHADER_SUITE_IMGUI_PATH}/examples/imgui_impl_opengl3.cpp")
set(CSL_SHADER_SUITE_IMGUI_INCLUDE ${CSL_SHADER_SUITE_IMGUI_PATH})

### GLFW
set(CSL_SHADER_SUITE_GLFW_PATH "${CSL_SHADER_SUITE_EXTLIBS_PATH}/glfw/")
set(CSL_SHADER_SUITE_GLFW_INCLUDE "${CSL_SHADER_SUITE_GLFW_PATH}/include/")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_INSTALL OFF CACHE BOOL "")
set(GLFW_DOCUMENT_INTERNALS OFF CACHE BOOL "")
add_subdirectory(${CSL_SHADER_SUITE_GLFW_PATH})

add_executable(CSL_ShaderSuite ${ShaderSuiteApp} ${Shaders} ${CSL_CORE} ${CSL_CORE_v2} ${CSL_GLSL}
	${CSL_SHADER_SUITE_IMGUI_HEADERS} ${CSL_SHADER_SUITE_IMGUI_SRC} ${CSL_SHADER_SUITE_GLAD_SRC}
)

target_include_directories(CSL_ShaderSuite PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR} ${CSL_CORE_INCLUDE} ${CSL_SHADER_SUITE_SHADERS_INCLUDE} ${CSL_SHADER_SUITE_IMGUI_INCLUDE} ${CSL_SHADER_SUITE_GLAD_INCLUDE} ${CSL_SHADER_SUITE_GLFW_INCLUDE}
)

target_link_libraries(CSL_ShaderSuite PRIVATE
	glfw
)
