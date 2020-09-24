#pragma once

#include "../Listeners.hpp"

#include "Types.hpp"
#include "BuiltIns.hpp"
#include "Qualifiers.hpp"
#include "../Structs.hpp"

namespace csl {

	namespace glsl {

		enum class GLSLversion : std::size_t {
			_420 = 420,
			_430 = 430
		};

		enum class ShaderType : std::size_t {
			Vertex,
			TessellationControl,
			TessellationEvaluation,
			Geometry,
			Fragment,
			Compute
		};

		template<ShaderType type, GLSLversion version>
		struct BuiltInRegisters;

		template<ShaderType type, GLSLversion version>
		class ShaderGLSL final : protected ShaderController {

		public:
			ShaderGLSL();
			ShaderGLSL(ShaderGLSL&& other) : ShaderController(std::move(other)) {
				//std::cout << "ShaderGLSL(ShaderGLSL&&)" << std::endl;
			}
			ShaderGLSL& operator=(ShaderGLSL&& other) {
				static_cast<ShaderController&>(*this) = std::move(other);
				return *this;
			}

			~ShaderGLSL();

			template<typename Data>
			void print_imgui(Data& data) {
				BuiltInRegisters<type, version>::template call(data.glsl_data);
				ShaderController::template print_imgui<csl::Dummy>(data);
			}

			template<typename Data>
			void print_glsl(Data& data) {
				BuiltInRegisters<type, version>::template call(data);
				data << get_header();
				ShaderController::template print_glsl<csl::Dummy>(data);
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

			CSL_PP_BUILTIN_UNNAMED_INTERFACE_BLOCK(Out, gl_PerVertex, gl_PerVertexVert, ObjFlags::BuiltInConstructor,
				(vec4, gl_Position),
				(Float, gl_PointSize),
				((Qualify<Float, Array<0>>), gl_ClipDistance)
			);

			inline const Qualify<Int, In> gl_VertexID("gl_VertexID", ObjFlags::BuiltInConstructor);
			inline const Qualify<Int, In> gl_InstanceID("gl_InstanceID", ObjFlags::BuiltInConstructor);
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

			inline const Qualify<Int, In> gl_PatchVerticesIn("gl_PatchVerticesIn");
			inline const Qualify<Int, In> gl_PrimitiveID("gl_PrimitiveID");
			inline const Qualify<Int, In> gl_InvocationID("gl_InvocationID");

			CSL_PP_BUILTIN_INTERFACE_BLOCK((In, Array<0>), gl_PerVertex, gl_PerVertexTCSin, gl_in,
				(vec4, gl_Position),
				(Float, gl_PointSize),
				((Qualify<Float, Array<0>>), gl_ClipDistance)
			);

			inline Qualify<Float, Out, Array<4>> gl_TessLevelOuter("gl_TessLevelOuter");
			inline Qualify<Float, Out, Float, Array<2>> gl_TessLevelInner("gl_TessLevelInner");

			CSL_PP_BUILTIN_INTERFACE_BLOCK((Out, Array<0>), gl_PerVertex, gl_PerVertexTCSout, gl_out,
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

			inline const Qualify<vec3, In> gl_TessCoord("gl_TessCoord");
			inline const Qualify<Int, In> gl_PatchVerticesIn("gl_PatchVerticesIn");
			inline const Qualify<Int, In> gl_PrimitiveID("gl_PrimitiveID");

			inline const Qualify<Float, In, Array<4>> gl_TessLevelOuter("gl_TessLevelOuter");
			inline const Qualify<Float, In, Float, Array<2>> gl_TessLevelInner("gl_TessLevelInner");

			CSL_PP_BUILTIN_INTERFACE_BLOCK((Out, Array<0>), gl_PerVertex, gl_PerVertexTEVin, gl_in,
				(vec4, gl_Position),
				(Float, gl_PointSize),
				((Qualify<Float, Array<0>>), gl_ClipDistance)
			);

			CSL_PP_BUILTIN_UNNAMED_INTERFACE_BLOCK(Out, gl_PerVertex, gl_PerVertexTEVout, ObjFlags::BuiltInConstructor,
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

			CSL_PP_BUILTIN_INTERFACE_BLOCK((In, Array<0>), gl_PerVertex, gl_PerVertexGeomIn, gl_in,
				(vec4, gl_Position),
				(Float, gl_PointSize),
				((Qualify<vec4, Array<0>>), gl_ClipDistance)
			);

			CSL_PP_BUILTIN_UNNAMED_INTERFACE_BLOCK(Out, gl_PerVertex, gl_PerVertexGeom, ObjFlags::BuiltInConstructor,
				(vec4, gl_Position),
				(Float, gl_PointSize),
				((Qualify<vec4, Array<0>>), gl_ClipDistance)
			);

			inline void EmitVertex() {
				listen().add_statement<SpecialStatement<csl::EmitVertexI>>();
			}

			inline void EndPrimitive() {
				listen().add_statement<SpecialStatement<csl::EndPrimitiveI>>();
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

			inline const Qualify<vec4, In> gl_FragCoord("gl_FragCoord", ObjFlags::BuiltInConstructor);
			inline const Qualify<Bool, In> gl_FrontFacing("gl_FrontFacing", ObjFlags::BuiltInConstructor);
			inline const Qualify<vec2, In> gl_PointCoord("gl_PointCoord", ObjFlags::BuiltInConstructor);

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

		namespace compute_common {
			using namespace shader_common;

			inline const Qualify<uvec3, In> gl_NumWorkGroups("gl_NumWorkGroups", ObjFlags::BuiltInConstructor);
			inline const Qualify<uvec3, In> gl_WorkGroupID("gl_WorkGroupID", ObjFlags::BuiltInConstructor);
			inline const Qualify<uvec3, In> gl_LocalInvocationID("gl_LocalInvocationID", ObjFlags::BuiltInConstructor);
			inline const Qualify<uvec3, In> gl_GlobalInvocationID("gl_GlobalInvocationID", ObjFlags::BuiltInConstructor);
			inline const Qualify<Uint, In> gl_LocalInvocationIndex("gl_LocalInvocationIndex", ObjFlags::BuiltInConstructor);

		}

		template<GLSLversion version>
		struct BuiltInRegisters<ShaderType::Compute, version> {
			template<typename Data>
			static void call(Data& data) {
				data.register_builtins(
					compute_common::gl_NumWorkGroups,
					compute_common::gl_WorkGroupID,
					compute_common::gl_LocalInvocationID,
					compute_common::gl_GlobalInvocationID,
					compute_common::gl_LocalInvocationIndex
				);
			}
		};

		namespace vert_420
		{
			using namespace csl;
			using namespace glsl_420;
			using namespace vert_common;

			using Shader = ShaderGLSL<ShaderType::Vertex, GLSLversion::_420>;
		}

		namespace tcs_420
		{
			using namespace csl;
			using namespace glsl_420;
			using namespace tcs_common;

			using Shader = ShaderGLSL<ShaderType::TessellationControl, GLSLversion::_420>;
		}

		namespace tev_420
		{
			using namespace csl;
			using namespace glsl_420;
			using namespace tev_common;

			using Shader = ShaderGLSL<ShaderType::TessellationEvaluation, GLSLversion::_420>;
		}

		namespace geom_420
		{

			using namespace csl;
			using namespace glsl_420;
			using namespace geom_common;

			using Shader = ShaderGLSL<ShaderType::Geometry, GLSLversion::_420>;
		}

		namespace frag_420
		{
			using namespace csl;
			using namespace glsl_420;
			using namespace frag_common;

			using Shader = ShaderGLSL<ShaderType::Fragment, GLSLversion::_420>;
		}

		namespace compute_430
		{
			using namespace csl;
			using namespace glsl_430;
			using namespace compute_common;

			using Shader = ShaderGLSL<ShaderType::Compute, GLSLversion::_430>;
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