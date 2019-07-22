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

		CSL_PP_BUILT_IN_NAMED_INTERFACE(In<>, gl_PerVertexIn, gl_in, 0,
			(vec4) gl_Position,
			(Float) gl_PointSize,
			(GetArray<Float>::Size<0>) gl_ClipDistance
		);

		CSL_PP_BUILT_IN_UNNAMED_INTERFACE(Out<>, gl_PerVertexOut, , ,
			(vec4) gl_Position,
			(Float) gl_PointSize,
			(GetArray<Float>::Size<0>) gl_ClipDistance
		);

		template<typename L> struct GeometryIn;
		template<typename ... LQualifiers> struct GeometryIn<Layout<LQualifiers...> > {
			static void declareIn() {
				Qualifier<EMPTY_QUALIFIER, EmptyType, Layout<LQualifiers...>> in("in");
			}
			static void declareOut() {
				Qualifier<EMPTY_QUALIFIER, EmptyType, Layout<LQualifiers...>> out("out");
			}
		};

		template<typename Layout> inline void in() {
			GeometryIn<Layout>::declareIn();
		}

		template<typename Layout> inline void out() {
			GeometryIn<Layout>::declareOut();
		}

		inline void EmitVertex() {
			listen().add_statement<EmitVertexInstruction>();
		}

		inline void EndPrimitive() {
			listen().add_statement<EndPrimitiveInstruction>();
		}
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

	namespace geom_330 {
		using namespace csl;
		using namespace geom_common;
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
