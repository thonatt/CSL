#pragma once

#include "Layouts.h"
#include "Samplers.h"
#include "BuiltInFunctions.h"
#include "StructHelpers.h"

namespace csl {

	namespace frag_common {

		const In<vec4> gl_FragCoord("gl_FragCoord", DISABLED);
		const In<Bool> gl_FrontFacing("gl_FrontFacing", DISABLED);
		const In<vec2> gl_PointCoord("gl_PointCoord", DISABLED);

		static Out<Float> gl_FragDepth("gl_FragDepth", DISABLED);

		inline void Discard() {
			listen().add_statement<DiscardStatement>();
		}
	}

	namespace geom_common {

		CSL_PP_BUILT_IN_NAMED_INTERFACE(In<>, gl_PerVertex, gl_in, 0,
			(vec4) gl_Position,
			(Float) gl_PointSize,
			(GetArray<Float>::Size<0>) gl_ClipDistance
		);

	}

	namespace vert_common {

		CSL_PP_BUILT_IN_UNNAMED_INTERFACE(Out<>, gl_PerVertex, , ,
			(vec4) gl_Position,
			(Float) gl_PointSize,
			(GetArray<Float>::Size<0>) gl_ClipDistance
		);

		const In<Int> gl_VertexID("gl_VertexID", DISABLED);
		const In<Int> gl_InstanceID("gl_InstanceID", DISABLED);
	}

	namespace vert_330 {
		using namespace csl;
		using namespace vert_common;
		using namespace glsl_330;

		using Shader = ShaderWrapper<GLSL_330>;
	}

	namespace frag_330 {
		using namespace csl;
		using namespace frag_common;
		using namespace glsl_330;

		using Shader = ShaderWrapper<GLSL_330>;
	}

	namespace frag_410 {
		using namespace csl;
		using namespace frag_common;
		using namespace glsl_410;

		using Shader = ShaderWrapper<GLSL_410>;
	}


	namespace vert_430 {
		using namespace csl;
		using namespace vert_common;
		using namespace glsl_430;

		using Shader = ShaderWrapper<GLSL_430>;
	}

	namespace frag_430 {
		using namespace csl;
		using namespace frag_common;
		using namespace glsl_430;

		using Shader = ShaderWrapper<GLSL_430>;
	}

	namespace frag_450 {
		using namespace csl;
		using namespace frag_common;
		using namespace glsl_450;

		using Shader = ShaderWrapper<GLSL_450>;
	}

} //namespace csl
