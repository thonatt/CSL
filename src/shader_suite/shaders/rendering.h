#pragma once

#include "v2/Listeners.hpp"
#include "v2/Structs.hpp"

#include <v2/glsl/Shaders.hpp>


/**
 Shader examples with rendering demos
 */

v2::glsl::frag_420::Shader textured_mesh_frag();

v2::glsl::frag_420::Shader multiple_outputs_frag();

v2::glsl::geom_420::Shader geometric_normals();

v2::glsl::frag_420::Shader single_color_frag();

v2::glsl::tcs_420::Shader tessellation_control_shader_example();

v2::glsl::tev_420::Shader tessellation_evaluation_shader_example();

v2::glsl::frag_420::Shader phong_shading_frag();

v2::glsl::vert_420::Shader interface_vertex_shader();

v2::glsl::vert_420::Shader screen_quad_vertex_shader();