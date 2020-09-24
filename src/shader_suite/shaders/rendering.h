#pragma once

#include <include/csl.hpp>

/**
 Shader examples with rendering demos
 */

csl::glsl::frag_420::Shader textured_mesh_frag();

csl::glsl::frag_420::Shader multiple_outputs_frag();

csl::glsl::geom_420::Shader geometric_normals();

csl::glsl::frag_420::Shader single_color_frag();

csl::glsl::tcs_420::Shader tessellation_control_shader_example();

csl::glsl::tev_420::Shader tessellation_evaluation_shader_example();

csl::glsl::frag_420::Shader phong_shading_frag();

csl::glsl::vert_420::Shader interface_vertex_shader();

csl::glsl::vert_420::Shader screen_quad_vertex_shader();