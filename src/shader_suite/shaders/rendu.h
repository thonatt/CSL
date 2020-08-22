#pragma once


#include "v2/Listeners.hpp"
#include "v2/Structs.hpp"

#include <v2/glsl/Shaders.hpp>

/**
 Shaders from the Rendu graphics engine, by Simon Rodriguez.
 https://github.com/kosua20/Rendu
 */

v2::glsl::compute_430::Shader scattering_lookup_table();

v2::glsl::frag_420::Shader atmosphere_rendering();