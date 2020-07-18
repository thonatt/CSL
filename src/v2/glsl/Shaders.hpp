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
		}

		namespace vert_common {
			using namespace shader_common;

			static Qualify<vec4, Out> gl_Position("gl_Position", ObjFlags::BuiltInConstructor);
			static Qualify<Float, Out> gl_PointSize("gl_PointSize", ObjFlags::BuiltInConstructor);
			static Qualify<Float, Out, Array<0> > gl_ClipDistance("gl_ClipDistance", ObjFlags::BuiltInConstructor);

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