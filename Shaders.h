#pragma once

#include "Layouts.h"
#include "Samplers.h"
#include "BuiltInFunctions.h"
#include "StructHelpers.h"

namespace fragment_shader {

	static const In<vec4> gl_FragCoord("gl_FragCoord", DISABLED);
	static const In<Bool> gl_FrontFacing("gl_FrontFacing", DISABLED);
	static const In<vec2> gl_PointCoord("gl_PointCoord", DISABLED);

	static Out<Float> gl_FragDepth("gl_FragDepth", DISABLED);
};


namespace frag_330 {
	using namespace fragment_shader;
	using namespace glsl_330;

	using Shader = ShaderWrapper<GLSL_330>;

}
namespace frag_450 {
	using namespace fragment_shader;
	using namespace glsl_450;

	using Shader = ShaderWrapper<GLSL_450>;
}
