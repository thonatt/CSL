#pragma once

#include "include/Listeners.hpp"
#include "include/Structs.hpp"

#include <include/glsl/Shaders.hpp>

#include <include/imgui/ToImGui.hpp>


/**
 Shaders from the Dolphin emulator.
 https://github.com/dolphin-emu/dolphin
 Extracted version (generated on the fly in practice):
 https://gist.github.com/phire/25181a9bfd957ac68ea8c74afdd9e9e1
 */

v2::glsl::vert_420::Shader dolphin_ubershader_vertex();

v2::glsl::frag_420::Shader dolphin_ubershader_fragment();

