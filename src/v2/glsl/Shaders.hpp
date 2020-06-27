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
			Fragment
		};

		template<ShaderType type, GLSLversion version>
		struct BuiltInRegisters;

		template<ShaderType type, GLSLversion version>
		class ShaderGLSL : private ShaderController {

		public:
			ShaderGLSL();
			~ShaderGLSL();

			template<typename Data>
			void print_debug(Data& data) {
				ShaderController::print_debug(data);
			}

			template<typename Data>
			void print_imgui(Data& data) {
				ShaderController::print_imgui(data);
			}

			template<typename Data>
			void print_glsl(Data& data) {
				BuiltInRegisters<type, version>::call(data);
				data << get_header();
				data.endl();
				ShaderController::print_glsl(data);
			}

			template<typename F>
			void main(F&& f) {
				ShaderController::main(std::forward<F>(f));
			}

			const ShaderController& get_base() const {
				return static_cast<const ShaderController&>(*this);
			}

		private:

			static std::string get_header();
		};

		namespace shader_common
		{
		}

		namespace vert_common {
			using namespace shader_common;

			Qualify<vec4, Out> gl_Position("gl_Position", ObjFlags::BuiltIn);
			Qualify<Float, Out> gl_PointSize("gl_PointSize", ObjFlags::BuiltIn);
			Qualify<Float, Out, Array<0> > gl_ClipDistance("gl_ClipDistance", ObjFlags::BuiltIn);

			const Qualify<Int, In> gl_VertexID("gl_VertexID", ObjFlags::BuiltIn);
			const Qualify<Int, In> gl_InstanceID("gl_InstanceID", ObjFlags::BuiltIn);
		}

		template<GLSLversion version>
		struct BuiltInRegisters<ShaderType::Vertex, version> {
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

		namespace frag_common {

			using namespace shader_common;

			const Qualify<vec4, In> gl_FragCoord("gl_FragCoord", ObjFlags::BuiltIn);
			const Qualify<Bool, In> gl_FrontFacing("gl_FrontFacing", ObjFlags::BuiltIn);
			const Qualify<vec2, In> gl_PointCoord("gl_PointCoord", ObjFlags::BuiltIn);

			static Qualify<Float, Out> gl_FragDepth("gl_FragDepth", ObjFlags::BuiltIn);

			inline void _csl_only_available_in_discard_context_() {
				listen().add_statement<DiscardStatement>();
			}
		}

		template<GLSLversion version>
		struct BuiltInRegisters<ShaderType::Fragment, version> {
			static void call(GLSLData& data) {
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