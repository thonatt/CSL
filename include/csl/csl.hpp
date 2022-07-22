#pragma once

#define CSL_MAJOR_VERSION 0
#define CSL_MINOR_VERSION 2
#define CSL_VERSION 100*CSL_MAJOR_VERSION + CSL_MINOR_VERSION

#define CSL_GLSL
#ifdef CSL_NO_GLSL
#undef CSL_GLSL
#endif 

#define CSL_IMGUI
#ifdef CSL_NO_IMGUI
#undef CSL_IMGUI
#endif

#include <csl/Listeners.hpp>
#include <csl/Structs.hpp>
#include <csl/Samplers.hpp>

#ifdef CSL_GLSL
#include <csl/glsl/BuiltIns.hpp>
#include <csl/glsl/Shaders.hpp>
#include <csl/glsl/ToGLSL.hpp>
#endif //  CSL_GLSL

#ifdef CSL_IMGUI
#include <csl/imgui/ToImGui.hpp>
#endif // CSL_IMGUI

