#pragma once

#include "../Listeners.hpp"

#include "Types.hpp"
#include "BuiltIns.hpp"
#include "Qualifiers.hpp"

namespace v2 {

	namespace glsl {

		enum class GLSLversion : std::size_t {
			_420 = 420
		};

		enum class ShaderType : std::size_t {
			Vertex,
			TessellationControl,
			TessellationEvaluation,
			Geometry,
			Fragment
		};

		template<ShaderType type, GLSLversion version>
		struct BuiltInRegisters;

		template<ShaderType type, GLSLversion version>
		class ShaderGLSL : protected ShaderController {

		public:
			ShaderGLSL();
			ShaderGLSL(ShaderGLSL&& other) : ShaderController(std::move(other)) {
				//std::cout << "ShaderGLSL(ShaderGLSL&&)" << std::endl;
			}

			~ShaderGLSL();

			template<typename Delayed, typename Data>
			void print_debug(Data& data) {
				ShaderController::print_debug<v2::Dummy>(data);
			}

			template<typename Data>
			void print_imgui(Data& data) {
				ShaderController::print_imgui<v2::Dummy>(data);
			}

			template<typename Data>
			void print_glsl(Data& data) {
				BuiltInRegisters<type, version>::call(data);
				data << get_header();
				data.endl();
				ShaderController::print_glsl<v2::Dummy>(data);
			}

			template<typename F>
			void main(F&& f) {
				ShaderController::main(std::forward<F>(f));
			}

			const ShaderController& get_base() const {
				return static_cast<const ShaderController&>(*this);
			}
			ShaderController& get_base() {
				return static_cast<ShaderController&>(*this);
			}

		private:

			static std::string get_header();
		};

		namespace shader_common
		{
			template<typename ...Qs>
			void in() {
				listen().add_statement<SpecialStatement<InInstruction<Qs...>>>();
			}

			template<typename ...Qs>
			void out() {
				listen().add_statement<SpecialStatement<OutInstruction<Qs...>>>();
			}

		}

		namespace vert_common {
			using namespace shader_common;

			CSL_PP2_BUILTIN_UNNAMED_INTERFACE_BLOCK(Out, gl_PerVertex, gl_PerVertexVert, ObjFlags::BuiltInConstructor,
				(vec4, gl_Position),
				(Float, gl_PointSize),
				((Qualify<Float, Array<0>>), gl_ClipDistance)
			);

			static const Qualify<Int, In> gl_VertexID("gl_VertexID", ObjFlags::BuiltInConstructor);
			static const Qualify<Int, In> gl_InstanceID("gl_InstanceID", ObjFlags::BuiltInConstructor);
		}

		template<GLSLversion version>
		struct BuiltInRegisters<ShaderType::Vertex, version> {

			template<typename Data>
			static void call(Data& data) {
				data.register_builtins(
					vert_common::gl_Position,
					vert_common::gl_PointSize,
					vert_common::gl_ClipDistance,
					vert_common::gl_VertexID,
					vert_common::gl_InstanceID
				);
			}
		};


		namespace tcs_common {
			using namespace shader_common;

			static const Qualify<Int, In> gl_PatchVerticesIn("gl_PatchVerticesIn");
			static const Qualify<Int, In> gl_PrimitiveID("gl_PrimitiveID");
			static const Qualify<Int, In> gl_InvocationID("gl_InvocationID");

			CSL_PP2_BUILTIN_INTERFACE_BLOCK((In, Array<0>), gl_PerVertex, gl_PerVertexTCSin, gl_in,
				(vec4, gl_Position),
				(Float, gl_PointSize),
				((Qualify<Float, Array<0>>), gl_ClipDistance)
			);

			static Qualify<Float, Out, Array<4>> gl_TessLevelOuter("gl_TessLevelOuter");
			static Qualify<Float, Out, Float, Array<2>> gl_TessLevelInner("gl_TessLevelInner");

			CSL_PP2_BUILTIN_INTERFACE_BLOCK((Out, Array<0>), gl_PerVertex, gl_PerVertexTCSout, gl_out,
				(vec4, gl_Position),
				(Float, gl_PointSize),
				((Qualify<Float, Array<0>>), gl_ClipDistance)
			);
		}

		template<GLSLversion version>
		struct BuiltInRegisters<ShaderType::TessellationControl, version> {
			template<typename Data>
			static void call(Data& data) {
				data.register_builtins(
					tcs_common::gl_PatchVerticesIn,
					tcs_common::gl_PrimitiveID,
					tcs_common::gl_InvocationID,
					tcs_common::gl_in,
					tcs_common::gl_TessLevelOuter,
					tcs_common::gl_TessLevelInner,
					tcs_common::gl_out
				);
			}
		};

		namespace tev_common {
			using namespace shader_common;

			static const Qualify<vec3, In> gl_TessCoord("gl_TessCoord");
			static const Qualify<Int, In> gl_PatchVerticesIn("gl_PatchVerticesIn");
			static const Qualify<Int, In> gl_PrimitiveID("gl_PrimitiveID");

			static const Qualify<Float, In, Array<4>> gl_TessLevelOuter("gl_TessLevelOuter");
			static const Qualify<Float, In, Float, Array<2>> gl_TessLevelInner("gl_TessLevelInner");

			CSL_PP2_BUILTIN_INTERFACE_BLOCK((Out, Array<0>), gl_PerVertex, gl_PerVertexTEVin, gl_in,
				(vec4, gl_Position),
				(Float, gl_PointSize),
				((Qualify<Float, Array<0>>), gl_ClipDistance)
			);

			CSL_PP2_BUILTIN_UNNAMED_INTERFACE_BLOCK(Out, gl_PerVertex, gl_PerVertexTEVout, ObjFlags::BuiltInConstructor,
				(vec4, gl_Position),
				(Float, gl_PointSize),
				((Qualify<Float, Array<0>>), gl_ClipDistance)
			);
		}

		template<GLSLversion version>
		struct BuiltInRegisters<ShaderType::TessellationEvaluation, version> {
			template<typename Data>
			static void call(Data& data) {
				data.register_builtins(
					tev_common::gl_TessCoord,
					tev_common::gl_PatchVerticesIn,
					tev_common::gl_PrimitiveID,
					tev_common::gl_TessLevelOuter,
					tev_common::gl_TessLevelInner,
					tev_common::gl_in,
					tev_common::gl_Position,
					tev_common::gl_PointSize,
					tev_common::gl_ClipDistance
				);
			}
		};

		namespace geom_common {
			using namespace shader_common;

			CSL_PP2_BUILTIN_INTERFACE_BLOCK((In, Array<0>), gl_PerVertex, gl_PerVertexGeomIn, gl_in,
				(vec4, gl_Position),
				(Float, gl_PointSize),
				((Qualify<vec4, Array<0>>), gl_ClipDistance)
			);

			CSL_PP2_BUILTIN_UNNAMED_INTERFACE_BLOCK(Out, gl_PerVertex, gl_PerVertexGeom, ObjFlags::BuiltInConstructor,
				(vec4, gl_Position),
				(Float, gl_PointSize),
				((Qualify<vec4, Array<0>>), gl_ClipDistance)
			);

			inline void EmitVertex() {
				listen().add_statement<SpecialStatement<v2::EmitVertexI>>();
			}

			inline void EndPrimitive() {
				listen().add_statement<SpecialStatement<v2::EndPrimitiveI>>();
			}
		}

		template<GLSLversion version>
		struct BuiltInRegisters<ShaderType::Geometry, version> {

			template<typename Data>
			static void call(Data& data) {
				data.register_builtins(
					geom_common::gl_Position,
					geom_common::gl_PointSize,
					geom_common::gl_ClipDistance,
					geom_common::gl_in
				);
			}
		};

		namespace frag_common {
			using namespace shader_common;

			static const Qualify<vec4, In> gl_FragCoord("gl_FragCoord", ObjFlags::BuiltInConstructor);
			static const Qualify<Bool, In> gl_FrontFacing("gl_FrontFacing", ObjFlags::BuiltInConstructor);
			static const Qualify<vec2, In> gl_PointCoord("gl_PointCoord", ObjFlags::BuiltInConstructor);

			static Qualify<Float, Out> gl_FragDepth("gl_FragDepth", ObjFlags::BuiltInConstructor);

			inline void _csl_only_available_in_discard_context_() {
				listen().add_statement<SpecialStatement<Discard>>();
			}
		}

		template<GLSLversion version>
		struct BuiltInRegisters<ShaderType::Fragment, version> {

			template<typename Data>
			static void call(Data& data) {
				data.register_builtins(
					frag_common::gl_FragCoord,
					frag_common::gl_FrontFacing,
					frag_common::gl_PointCoord,
					frag_common::gl_FragDepth
				);
			}
		};

		namespace vert_420
		{
			using namespace v2;
			using namespace glsl_420;
			using namespace vert_common;

			using Shader = ShaderGLSL<ShaderType::Vertex, GLSLversion::_420>;
		}

		namespace tcs_420
		{
			using namespace v2;
			using namespace glsl_420;
			using namespace tcs_common;

			using Shader = ShaderGLSL<ShaderType::TessellationControl, GLSLversion::_420>;
		}

		namespace tev_420
		{
			using namespace v2;
			using namespace glsl_420;
			using namespace tev_common;

			using Shader = ShaderGLSL<ShaderType::TessellationEvaluation, GLSLversion::_420>;
		}

		namespace geom_420
		{

			using namespace v2;
			using namespace glsl_420;
			using namespace geom_common;

			using Shader = ShaderGLSL<ShaderType::Geometry, GLSLversion::_420>;
		}

		namespace frag_420
		{
			using namespace v2;
			using namespace glsl_420;
			using namespace frag_common;

			using Shader = ShaderGLSL<ShaderType::Fragment, GLSLversion::_420>;
		}


		template<ShaderType type, GLSLversion version>
		inline ShaderGLSL<type, version>::ShaderGLSL()
		{
			listen().current_shader = this;
		}

		template<ShaderType type, GLSLversion version>
		inline ShaderGLSL<type, version>::~ShaderGLSL()
		{
			if (listen().current_shader == this) {
				listen().current_shader = nullptr;
			}
		}

		template<ShaderType type, GLSLversion version>
		inline std::string ShaderGLSL<type, version>::get_header()
		{
			return "#version " + std::to_string(static_cast<std::size_t>(version));
		}

	}
}