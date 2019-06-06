#pragma once

#include "Layouts.h"
#include "Samplers.h"
#include "BuiltInFunctions.h"
#include "StructHelpers.h"

namespace frag_all {

	static const In<vec4> gl_FragCoord("gl_FragCoord", DISABLED);
	static const In<Bool> gl_FrontFacing("gl_FrontFacing", DISABLED);
	static const In<vec2> gl_PointCoord("gl_PointCoord", DISABLED);

	static Out<Float> gl_FragDepth("gl_FragDepth", DISABLED);


	inline void DiscardStatement() {
		listen().add_statement<DiscardeStatement>();
	}
}

namespace vert_all {

}

namespace vert_330 {
	using namespace vert_all;
	using namespace glsl_330;

	using Shader = ShaderWrapper<GLSL_330>;
}

namespace frag_330 {
	using namespace frag_all;
	using namespace glsl_330;

	using Shader = ShaderWrapper<GLSL_330>;
}

namespace frag_450 {
	using namespace frag_all;
	using namespace glsl_450;

	using Shader = ShaderWrapper<GLSL_450>;
}
