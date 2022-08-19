#pragma once

#include <csl/csl.hpp>

/**
 Shaders from the Rendu graphics engine, by Simon Rodriguez.
 https://github.com/kosua20/Rendu
 */

csl::glsl::compute_430::Shader scattering_lookup_table();

csl::glsl::frag_420::Shader atmosphere_rendering();