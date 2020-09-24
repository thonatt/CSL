#pragma once

#include <include/csl.hpp>

/**
 Shaders from the Dolphin emulator.
 https://github.com/dolphin-emu/dolphin
 Extracted version (generated on the fly in practice):
 https://gist.github.com/phire/25181a9bfd957ac68ea8c74afdd9e9e1
 */

csl::glsl::vert_420::Shader dolphin_ubershader_vertex();

csl::glsl::frag_420::Shader dolphin_ubershader_fragment();

