#pragma once


#include "include/Listeners.hpp"
#include "include/Structs.hpp"

#include <include/glsl/Shaders.hpp>
#include <include/imgui/ToImGui.hpp>

/**
 Shaderwave shader adapted specifically for the CSL project, by Simon Rodriguez.
 http://shadertoy.com/view/ttBGRD
 */
v2::glsl::frag_420::Shader shader_80();

/**
Fractal noise fragment shader by Inigo Quilez adatped from
https://www.shadertoy.com/view/XdXGW8
*/
v2::glsl::frag_420::Shader fractal_noise();