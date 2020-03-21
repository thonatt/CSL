#pragma once

#include "Listener.hpp"
#include "Layouts.hpp"
#include "Samplers.hpp"
#include "BuiltInFunctions.hpp"
#include "StructHelpers.hpp"

namespace csl {

	namespace core {

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

			const In<vec4> gl_FragCoord("gl_FragCoord", OpFlags::BUILT_IN);
			const In<Bool> gl_FrontFacing("gl_FrontFacing", OpFlags::BUILT_IN);
			const In<vec2> gl_PointCoord("gl_PointCoord", OpFlags::BUILT_IN);

			static Out<Float> gl_FragDepth("gl_FragDepth", OpFlags::BUILT_IN);

			inline void Discard() {
				listen().add_statement<DiscardStatement>();
			}
		}

		namespace geom_common {

			using namespace shader_common;

			CSL_PP_BUILT_IN_NAMED_INTERFACE(In<>, gl_PerVertex, gl_in, 0,
				(vec4) gl_Position,
				(Float) gl_PointSize,
				(Array<Float>::Size<0>) gl_ClipDistance
			);

			CSL_PP_BUILT_IN_UNNAMED_INTERFACE(Out<>, gl_PerVertex, , ,
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

			const In<Int> gl_VertexID("gl_VertexID", OpFlags::BUILT_IN);
			const In<Int> gl_InstanceID("gl_InstanceID", OpFlags::BUILT_IN);
		}

		namespace tesc_common {
			using namespace shader_common;
			
			const In<Int> gl_PatchVerticesIn("gl_PatchVerticesIn", OpFlags::BUILT_IN);
			const In<Int> gl_PrimitiveID("gl_PrimitiveID", OpFlags::BUILT_IN);
			const In<Int> gl_InvocationID("gl_InvocationID", OpFlags::BUILT_IN);

			const Array<Out<Float>, 4> gl_TessLevelOuter("gl_TessLevelOuter", OpFlags::BUILT_IN);
			const Array<Out<Float>, 2> gl_TessLevelInner("gl_TessLevelInner", OpFlags::BUILT_IN);

			CSL_PP_BUILT_IN_NAMED_INTERFACE(In<>, gl_PerVertexIn, gl_in, 0,
				(vec4) gl_Position,
				(Float) gl_PointSize,
				(Array<Float>::Size<0>) gl_ClipDistance
			);

			CSL_PP_BUILT_IN_NAMED_INTERFACE(Out<>, gl_PerVertexOut, gl_out, 0,
				(vec4) gl_Position,
				(Float) gl_PointSize,
				(Array<Float>::Size<0>) gl_ClipDistance
			);
		}

		namespace tese_common {
			using namespace shader_common;

			const In<vec3> gl_TessCoord("gl_TessCoord", OpFlags::BUILT_IN);
			const In<Int> gl_PatchVerticesIn("gl_PatchVerticesIn", OpFlags::BUILT_IN);
			const In<Int> gl_PrimitiveID("gl_PrimitiveID", OpFlags::BUILT_IN);

			const Array<In<Float>, 4> gl_TessLevelOuter("gl_TessLevelOuter", OpFlags::BUILT_IN);
			const Array<In<Float>, 2> gl_TessLevelInner("gl_TessLevelInner", OpFlags::BUILT_IN);

			CSL_PP_BUILT_IN_NAMED_INTERFACE(In<>, gl_PerVertexIn, gl_in, 0,
				(vec4) gl_Position,
				(Float) gl_PointSize,
				(Array<Float>::Size<0>) gl_ClipDistance
			);

			CSL_PP_BUILT_IN_UNNAMED_INTERFACE(Out<>, gl_PerVertexOut, gl_out, ,
				(vec4) gl_Position,
				(Float) gl_PointSize,
				(Array<Float>::Size<0>) gl_ClipDistance
			);

		}

		template<GLVersion version>
		struct IShader : ShaderBase {
			using Ptr = std::shared_ptr<IShader>;

			virtual std::string header() const {
				return "#version " + gl_version_str<version>();
			}
		};

		template<>
		struct IShader<SHADERTOY> : ShaderBase {
			using Ptr = std::shared_ptr<IShader>;

			virtual std::string header() const {
				return "";
			}
		};

		template<GLVersion version>
		struct ShaderWrapper
		{

			ShaderWrapper() {
				shader_ptr = std::make_shared<IShader<version>>();
				listen().currentShader = shader_ptr;
				listen().reset_counters();
			}

			template<typename F_Type>
			void main(const F_Type & f) {
				using ArgsTList = GetArgTList<F_Type>;
				static_assert(EqualList<ArgsTList, TList<>> || EqualList<ArgsTList, TList<void>>, "main function should not have arguments");
				Function<TList<void>, TList<F_Type>>("main", f);
			}

			std::string str() {
				if (shader_ptr) {
					return shader_ptr->str();
				} else {
					return "";
				}
			}

			typename IShader<version>::Ptr shader_ptr;
		};

	}

	namespace vert_330 {
		using namespace csl;
		using namespace core::vert_common;
		using namespace core::glsl_330;

		using Shader = core::ShaderWrapper<core::GLSL_330>;
	}

	namespace frag_330 {
		using namespace csl;
		using namespace core::frag_common;
		using namespace core::glsl_330;

		using Shader = core::ShaderWrapper<core::GLSL_330>;
	}

	namespace geom_330 {
		using namespace csl;
		using namespace core::geom_common;
		using namespace core::glsl_330;

		using Shader = core::ShaderWrapper<core::GLSL_330>;
	}

	namespace frag_410 {
		using namespace csl;
		using namespace core::frag_common;
		using namespace core::glsl_410;

		using Shader = core::ShaderWrapper<core::GLSL_410>;
	}

	namespace tesc_400 {
		using namespace csl;
		using namespace core::tesc_common;
		using namespace core::glsl_400;

		using Shader = core::ShaderWrapper<core::GLSL_400>;
	}

	namespace tese_400 {
		using namespace csl;
		using namespace core::tese_common;
		using namespace core::glsl_400;

		using Shader = core::ShaderWrapper<core::GLSL_400>;
	}

	namespace vert_430 {
		using namespace csl;
		using namespace core::vert_common;
		using namespace core::glsl_430;

		using Shader = core::ShaderWrapper<core::GLSL_430>;
	}

	namespace frag_430 {
		using namespace csl;
		using namespace core::frag_common;
		using namespace core::glsl_430;

		using Shader = core::ShaderWrapper<core::GLSL_430>;
	}

	namespace frag_450 {
		using namespace csl;
		using namespace core::frag_common;
		using namespace core::glsl_450;

		using Shader = core::ShaderWrapper<core::GLSL_450>;
	}

	namespace shadertoy {
		using namespace csl;
		using namespace core::glsl_450;
		using Shader = core::ShaderWrapper<core::SHADERTOY>;

		const Uniform<vec4> 
			iMouse("iMouse", core::OpFlags::BUILT_IN),
			iDate("iDate", core::OpFlags::BUILT_IN);

		const Uniform<vec3> iResolution("iResolution", core::OpFlags::BUILT_IN);

		const Uniform<Float> 
			iTime("iTime", core::OpFlags::BUILT_IN),
			iTimeDelta("iTimeDelta", core::OpFlags::BUILT_IN),
			iFrame("iFrame", core::OpFlags::BUILT_IN),
			iSampleRate("iSampleRate", core::OpFlags::BUILT_IN);

		const Array<Uniform<vec3>, 4> iChannelResolution("iChannelResolution", core::OpFlags::BUILT_IN);
		const Array<Uniform<Float>, 4> iChannelTime("iChannelTime", core::OpFlags::BUILT_IN);

		const Uniform<sampler2D> 
			iChannel0("iChannel0", core::OpFlags::BUILT_IN),
			iChannel1("iChannel1", core::OpFlags::BUILT_IN),
			iChannel2("iChannel2", core::OpFlags::BUILT_IN),
			iChannel3("iChannel3", core::OpFlags::BUILT_IN);
	}

} //namespace csl
