#pragma once

#include "Layouts.hpp"
#include "Samplers.hpp"
#include "BuiltInFunctions.hpp"
#include "StructHelpers.hpp"

namespace csl {

	namespace shader_common {
		template<typename L> struct ShaderInOut;
		template<typename ... LQualifiers> struct ShaderInOut<Layout<LQualifiers...> > {
			static void declareIn() {
				Qualifier<EMPTY_QUALIFIER, EmptyType, Layout<LQualifiers...>> in("in");
			}
			static void declareOut() {
				Qualifier<EMPTY_QUALIFIER, EmptyType, Layout<LQualifiers...>> out("out");
			}
		};

		template<typename Layout> inline void in() {
			ShaderInOut<Layout>::declareIn();
		}

		template<typename Layout> inline void out() {
			ShaderInOut<Layout>::declareOut();
		}
	}


	namespace frag_common {

		using namespace shader_common;

		const In<vec4> gl_FragCoord("gl_FragCoord", DISABLED);
		const In<Bool> gl_FrontFacing("gl_FrontFacing", DISABLED);
		const In<vec2> gl_PointCoord("gl_PointCoord", DISABLED);

		static Out<Float> gl_FragDepth("gl_FragDepth", DISABLED);

		inline void Discard() {
			listen().add_statement<DiscardStatement>();
		}
	}

	namespace geom_common {

		using namespace shader_common;

		CSL_PP_BUILT_IN_NAMED_INTERFACE(In<>, gl_PerVertexIn, gl_in, 0,
			(vec4) gl_Position,
			(Float) gl_PointSize,
			(Array<Float>::Size<0>) gl_ClipDistance
		);

		CSL_PP_BUILT_IN_UNNAMED_INTERFACE(Out<>, gl_PerVertexOut, , ,
			(vec4) gl_Position,
			(Float) gl_PointSize,
			(Array<Float>::Size<0>) gl_ClipDistance
		);

		inline void EmitVertex() {
			listen().add_statement<EmitVertexInstruction>();
		}

		inline void EndPrimitive() {
			listen().add_statement<EndPrimitiveInstruction>();
		}
	}

	namespace vert_common {

		using namespace shader_common;

		CSL_PP_BUILT_IN_UNNAMED_INTERFACE(Out<>, gl_PerVertex, , ,
			(vec4) gl_Position,
			(Float) gl_PointSize,
			(Array<Float>::Size<0>) gl_ClipDistance
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
