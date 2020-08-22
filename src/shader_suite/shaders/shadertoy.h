#pragma once


#include "v2/Listeners.hpp"
#include "v2/Structs.hpp"

#include <v2/glsl/Shaders.hpp>

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