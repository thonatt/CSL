#pragma once

#include "Layouts.h"
#include "Samplers.h"
#include "BuiltInFunctions.h"
#include "StructHelpers.h"

namespace csl {

	namespace frag_all {

		const In<vec4> gl_FragCoord("gl_FragCoord", DISABLED);
		const In<Bool> gl_FrontFacing("gl_FrontFacing", DISABLED);
		const In<vec2> gl_PointCoord("gl_PointCoord", DISABLED);

		static Out<Float> gl_FragDepth("gl_FragDepth", DISABLED);


		inline void Discard() {
			listen().add_statement<DiscardStatement>();
		}
	}

	namespace vert_all {

		//static GL_INTERFACE(Out, GLperVertex,
		//	(vec4) gl_Position,
		//	(Float) gl_PointSize, 
		//	(Array<Float>) gl_ClipDistance
		//) gl_PerVertex("gl_PerVertex", DISABLED);

		static Out<vec4> gl_Position("gl_Position", DISABLED);
		static Out<Float> gl_PointSize("gl_PointSize", DISABLED);
		//static Out<Array<Float>> gl_ClipDistance("gl_ClipDistance", DISABLED);

		const In<Int> gl_VertexID("gl_VertexID", DISABLED);
		const In<Int> gl_InstanceID("gl_InstanceID", DISABLED);
	}

	namespace vert_330 {
		using namespace csl;
		using namespace vert_all;
		using namespace glsl_330;

		using Shader = ShaderWrapper<GLSL_330>;
	}

	namespace frag_330 {
		using namespace csl;
		using namespace frag_all;
		using namespace glsl_330;

		using Shader = ShaderWrapper<GLSL_330>;
	}

	namespace frag_410 {
		using namespace csl;
		using namespace frag_all;
		using namespace glsl_410;

		using Shader = ShaderWrapper<GLSL_410>;
	}


	namespace vert_430 {
		using namespace csl;
		using namespace vert_all;
		using namespace glsl_430;

		using Shader = ShaderWrapper<GLSL_430>;
	}

	namespace frag_430 {
		using namespace csl;
		using namespace frag_all;
		using namespace glsl_430;

		using Shader = ShaderWrapper<GLSL_430>;
	}

	namespace frag_450 {
		using namespace csl;
		using namespace frag_all;
		using namespace glsl_450;

		using Shader = ShaderWrapper<GLSL_450>;
	}

} //namespace csl
