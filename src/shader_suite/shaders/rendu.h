#pragma once


#include "include/Listeners.hpp"
#include "include/Structs.hpp"

#include <include/glsl/Shaders.hpp>

/**
 Shaders from the Rendu graphics engine, by Simon Rodriguez.
 https://github.com/kosua20/Rendu
 */

v2::glsl::compute_430::Shader scattering_lookup_table();

v2::glsl::frag_420::Shader atmosphere_rendering();