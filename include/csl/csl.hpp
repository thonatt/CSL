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

#include "Listeners.hpp"
#include "Structs.hpp"
#include "Samplers.hpp"

#ifdef CSL_GLSL
#include "glsl/Shaders.hpp"
#include "glsl/BuiltIns.hpp"
#include "glsl/ToGLSL.hpp"
#endif //  CSL_GLSL

#ifdef CSL_IMGUI
#include "imgui/ToImGui.hpp"
#endif // CSL_IMGUI

