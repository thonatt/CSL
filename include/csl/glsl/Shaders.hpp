#pragma once

#include <csl/Controllers.hpp>
#include <csl/Listeners.hpp>
#include <csl/Structs.hpp>
#include <csl/Types.hpp>

#include <csl/glsl/BuiltIns.hpp>
#include <csl/glsl/Qualifiers.hpp>
#include <csl/glsl/Types.hpp>

namespace csl
{

	namespace glsl
	{

		enum class GLSLversion : std::size_t
		{
			_420 = 420,
			_430 = 430
		};

		enum class ShaderType : std::size_t
		{
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
		class ShaderGLSL final : public ShaderController
		{

		public:
			ShaderGLSL();
			ShaderGLSL(ShaderGLSL&& other) noexcept : ShaderController(std::move(other)) {
				//std::cout << "ShaderGLSL(ShaderGLSL&&)" << std::endl;
			}
			ShaderGLSL& operator=(ShaderGLSL&& other) noexcept {
				static_cast<ShaderController&>(*this) = std::move(other);
				return *this;
			}

			~ShaderGLSL();

			void print_imgui(ImGuiData& data) override
			{
				BuiltInRegisters<type, version>::call(data.glsl_data);
				ShaderController::print_imgui(data);
			}

			void print_glsl(GLSLData& data) override
			{
				BuiltInRegisters<type, version>::call(data);
				data << get_header() << "\n";
				ShaderController::print_glsl(data);
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
			void shader_stage_option()
			{
				context::get().add_statement<SpecialStatement<ShaderStageOption<Qs...>>>();
			}
		}

		namespace vert_common {
			using namespace shader_common;

			CSL_PP_BUILTIN_UNNAMED_INTERFACE_BLOCK(out, gl_PerVertex, gl_PerVertexVert, ObjFlags::BuiltInConstructor,
				(vec4, gl_Position),
				(Float, gl_PointSize),
				((Qualify<Float, Array<0>>), gl_ClipDistance)
			);

			inline const Qualify<in, Int> gl_VertexID("gl_VertexID", ObjFlags::BuiltInConstructor);
			inline const Qualify<in, Int> gl_InstanceID("gl_InstanceID", ObjFlags::BuiltInConstructor);
		}

		template<GLSLversion version>
		struct BuiltInRegisters<ShaderType::Vertex, version>
		{
			static void call(GLSLData& data) {
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

			inline const Qualify<in, Int> gl_PatchVerticesIn("gl_PatchVerticesIn");
			inline const Qualify<in, Int> gl_PrimitiveID("gl_PrimitiveID");
			inline const Qualify<in, Int> gl_InvocationID("gl_InvocationID");

			CSL_PP_BUILTIN_INTERFACE_BLOCK((in, Array<0>), gl_PerVertex, gl_PerVertexTCSin, gl_in,
				(vec4, gl_Position),
				(Float, gl_PointSize),
				((Qualify<Float, Array<0>>), gl_ClipDistance)
			);

			inline Qualify<out, Float, Array<4>> gl_TessLevelOuter("gl_TessLevelOuter");
			inline Qualify<out, Float, Array<2>> gl_TessLevelInner("gl_TessLevelInner");

			CSL_PP_BUILTIN_INTERFACE_BLOCK((out, Array<0>), gl_PerVertex, gl_PerVertexTCSout, gl_out,
				(vec4, gl_Position),
				(Float, gl_PointSize),
				((Qualify<Float, Array<0>>), gl_ClipDistance)
			);
		}

		template<GLSLversion version>
		struct BuiltInRegisters<ShaderType::TessellationControl, version>
		{
			static void call(GLSLData& data) {
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

			inline const Qualify<in, vec3> gl_TessCoord("gl_TessCoord");
			inline const Qualify<in, Int> gl_PatchVerticesIn("gl_PatchVerticesIn");
			inline const Qualify<in, Int> gl_PrimitiveID("gl_PrimitiveID");

			inline const Qualify<in, Float, Array<4>> gl_TessLevelOuter("gl_TessLevelOuter");
			inline const Qualify<in, Float, Array<2>> gl_TessLevelInner("gl_TessLevelInner");

			CSL_PP_BUILTIN_INTERFACE_BLOCK((out, Array<0>), gl_PerVertex, gl_PerVertexTEVin, gl_in,
				(vec4, gl_Position),
				(Float, gl_PointSize),
				((Qualify<Float, Array<0>>), gl_ClipDistance)
			);

			CSL_PP_BUILTIN_UNNAMED_INTERFACE_BLOCK(out, gl_PerVertex, gl_PerVertexTEVout, ObjFlags::BuiltInConstructor,
				(vec4, gl_Position),
				(Float, gl_PointSize),
				((Qualify<Float, Array<0>>), gl_ClipDistance)
			);
		}

		template<GLSLversion version>
		struct BuiltInRegisters<ShaderType::TessellationEvaluation, version>
		{
			static void call(GLSLData& data) {
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

			CSL_PP_BUILTIN_INTERFACE_BLOCK((in, Array<0>), gl_PerVertex, gl_PerVertexGeomIn, gl_in,
				(vec4, gl_Position),
				(Float, gl_PointSize),
				((Qualify<vec4, Array<0>>), gl_ClipDistance)
			);

			CSL_PP_BUILTIN_UNNAMED_INTERFACE_BLOCK(out, gl_PerVertex, gl_PerVertexGeom, ObjFlags::BuiltInConstructor,
				(vec4, gl_Position),
				(Float, gl_PointSize),
				((Qualify<vec4, Array<0>>), gl_ClipDistance)
			);

			inline void EmitVertex() {
				context::get().add_statement<SpecialStatement<csl::EmitVertexI>>();
			}

			inline void EndPrimitive() {
				context::get().add_statement<SpecialStatement<csl::EndPrimitiveI>>();
			}
		}

		template<GLSLversion version>
		struct BuiltInRegisters<ShaderType::Geometry, version>
		{
			static void call(GLSLData& data) {
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

			inline const Qualify<in, vec4> gl_FragCoord("gl_FragCoord", ObjFlags::BuiltInConstructor);
			inline const Qualify<in, Bool> gl_FrontFacing("gl_FrontFacing", ObjFlags::BuiltInConstructor);
			inline const Qualify<in, vec2> gl_PointCoord("gl_PointCoord", ObjFlags::BuiltInConstructor);

			static Qualify<out, Float> gl_FragDepth("gl_FragDepth", ObjFlags::BuiltInConstructor);

			inline void _csl_only_available_in_discard_context_() {
				context::get().add_statement<SpecialStatement<Discard>>();
			}
		}

		template<GLSLversion version>
		struct BuiltInRegisters<ShaderType::Fragment, version>
		{
			static void call(GLSLData& data) {
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

			inline const Qualify<in, uvec3> gl_NumWorkGroups("gl_NumWorkGroups", ObjFlags::BuiltInConstructor);
			inline const Qualify<in, uvec3> gl_WorkGroupID("gl_WorkGroupID", ObjFlags::BuiltInConstructor);
			inline const Qualify<in, uvec3> gl_LocalInvocationID("gl_LocalInvocationID", ObjFlags::BuiltInConstructor);
			inline const Qualify<in, uvec3> gl_GlobalInvocationID("gl_GlobalInvocationID", ObjFlags::BuiltInConstructor);
			inline const Qualify<in, Uint> gl_LocalInvocationIndex("gl_LocalInvocationIndex", ObjFlags::BuiltInConstructor);

		}

		template<GLSLversion version>
		struct BuiltInRegisters<ShaderType::Compute, version>
		{
			static void call(GLSLData& data) {
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
			context::g_current_shader = this;
		}

		template<ShaderType type, GLSLversion version>
		inline ShaderGLSL<type, version>::~ShaderGLSL()
		{
			if (context::g_current_shader == this)
				context::g_current_shader = nullptr;
		}

		template<ShaderType type, GLSLversion version>
		inline std::string ShaderGLSL<type, version>::get_header()
		{
			return "#version " + std::to_string(static_cast<std::size_t>(version));
		}

	}
}