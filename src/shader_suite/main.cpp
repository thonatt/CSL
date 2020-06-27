#include <shaders/80_s_shader.h>
#include <shaders/dolphin.h>
#include <shaders/rendu_compilation.h>
#include <shaders/ogl4_sl_cookbook.h>
#include <shaders/examples.h>

#include "tests.hpp"

#include "v2/Listeners.hpp"
#include "v2/Structs.hpp"

#include <iostream>
#include <chrono>
#include <filesystem>
#include <memory>

#include "pico_gl.hpp"

#include "v2/ToDebug.hpp"
#include "v2/glsl/Shaders.hpp"
#include "v2/Samplers.hpp"
#include "v2/glsl/ToGLSL.hpp"
#include "v2/glsl/BuiltIns.hpp"
#include "ToImGui.hpp"



using Clock = std::chrono::high_resolution_clock;

template<typename F>
double get_timing(F&& f)
{
	auto start = Clock::now();
	f();
	return std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - start).count() / 1000.0;
}

struct ShaderExample {

	template<typename ShaderCreation>
	void init_shader(ShaderCreation&& f)
	{
		using namespace v2;

		auto start = Clock::now();
		auto shader = f();
		m_generation_timing = std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - start).count() / 1000.0;

		m_debug_timing = get_timing([&]
		{
			DebugData debug_data;
			shader.print_debug(debug_data);
			m_debug_str = debug_data.stream.str();
		});

		m_glsl_timing = get_timing([&]
		{
			GLSLData glsl_data;
			shader.print_glsl(glsl_data);
			m_glsl_str = glsl_data.stream.str();
		});

		m_controller = shader.get_base();


	}

	GLProgram m_program;
	std::string m_debug_str, m_glsl_str;
	v2::ShaderController m_controller;
	double m_generation_timing, m_glsl_timing, m_debug_timing;
};

auto test_frag_ops()
{
	using namespace v2::glsl::frag_420;
	Shader shader;

	CSL2_STRUCT(Plop,
		(vec3)v,
		(Float)f
	);

	CSL2_STRUCT(BigPlop,
		(Plop)plop,
		(Float)g
	);

	auto ffff = define_function<vec3>("f", [&](vec3 aa = "a", vec3 bb = "b") {
		BigPlop b;
		b.plop.v* BigPlop().plop.f;

		CSL_IF(true) {
			b.plop.v = 2.0 * b.plop.v;
		} CSL_ELSE_IF(false) {
			b.plop.v = 3.0 * b.plop.v;
		} CSL_ELSE{
			b.plop.v = 4.0 * b.plop.v;
		}
	});

	using T = vec3;

	Qualify<T, Uniform, Array<3, 2>> b;

	Qualify<sampler2D, Uniform> s;

	shader.main([&]
	{
		using namespace v2::swizzles::xyzw;
		//b[2][1][x, y, z][z, z, y];
		//ffff(b[2][1], b[2][1]) + ffff(b[2][1], b[2][1]);

		mat3 m("m");
		vec3 v("v");
		Float f("f");

		m* v;
		f* m;
		m* f;
		f* v;
		v* f;
		m* m;
		v* v;
		f* f;

		m + m;
		v + v;
		f + f;
		m + f;
		f + m;
		v + f;
		f + v;

		m - m;
		v - v;
		f - f;
		m - f;
		f - m;
		v - f;
		f - v;

		m / m;
		v / v;
		f / f;
		m / f;
		f / m;
		v / f;
		f / v;

		(f * (v + v))[x, y, z];
		(f + f * f)* (f * f + f);

		gl_FragCoord[x, y, z] * gl_FragDepth;

		greaterThan(cos(dFdx(f)), sin(dFdy(f)));

		texture(s, v[x, y]);
	});

	return shader;
}

auto test_vert_quad()
{
	using namespace v2::glsl::vert_420;
	using namespace v2::swizzles::rgba;
	Shader shader;

	Qualify<vec2, Layout<Location<0>>, Out> uv("uv");

	shader.main([&]
	{
		Float x = mod(Float((gl_VertexID + 2) / 3), 2.0);
		Float y = mod(Float((gl_VertexID + 1) / 3), 2.0);

		gl_Position = vec4(-1.0 + x * 2.0, -1.0 + y * 2.0, 0.0, 1.0);
		uv = vec2(x, y);
	});

	return shader;
}

auto test_frag_quad()
{
	using namespace v2::glsl::frag_420;
	using namespace v2::swizzles::rgba;
	Shader shader;

	Qualify<vec2, Layout<Location<0>>, In> uv("uv");
	Qualify<vec4, Out> out_color("out_color");

	Qualify<Float, Uniform> time("time");

	shader.main([&]
	{
		out_color = vec4(mod(time / vec2(1.3, 1.7) + 4.5 * uv, 1.0), 0.0, 1.0);
	});

	return shader;
}

auto test_80() {

	using namespace v2::glsl::frag_420;
	using namespace v2::swizzles::all;
	Shader shader;

	Qualify<sampler2D, Layout<Binding<0>>, Uniform> iChannel0("iChannel0");

	/// Noise helpers.	
	auto noise = define_function<Float, vec4>("noise",
		[](Float p) {
		Float fl = floor(p);
		Float fc = fract(p);
		Float rand0 = fract(sin(fl) * 43758.5453123);
		Float rand1 = fract(sin(fl + 1.0) * 43758.5453123);
		CSL_RETURN(mix(rand0, rand1, fc));
	},
		[](vec4 p) {
		vec4 fl = floor(p);
		vec4 fc = fract(p);
		vec4 rand0 = fract(sin(fl) * 43758.5453123);
		vec4 rand1 = fract(sin(fl + 1.0) * 43758.5453123);
		CSL_RETURN(mix(rand0, rand1, fc));
	});
	
	auto hash = define_function<Float>([](vec2 p) {
		vec3 p3 = fract(p[x, y, x] * 0.2831);
		p3 += dot(p3, p3[y, z, x] + 19.19);
		CSL_RETURN(fract((p3[x] + p3[y]) * p3[z]));
	});

	/// Background utilities.
	auto stars = define_function<Float>([&](vec2 localUV = "localUV", Float starsDens = "starsDens", Float starsDist = "starsDist") {
		// Center and scale UVs.
		vec2 p = (localUV - 0.5) * starsDist;
		// Use thresholded high-frequency noise.
		Float brightness = smoothstep(1.0 - starsDens, 1.0, hash(floor(p)));
		// Apply soft transition between the stars and the background.
		const Float startsTh = 0.5;
		CSL_RETURN(smoothstep(startsTh, 0.0, length(fract(p) - 0.5)) * brightness);
	});

	auto segmentDistance = define_function<Float>([](vec2 p, vec2 a, vec2 b) {
		// Project the point on the segment.
		vec2 dir = b - a;
		Float len2 = dot(dir, dir);
		Float t = clamp(dot(p - a, dir) / len2, 0.0, 1.0);
		vec2 proj = a + t * dir;
		// Distance between the point and its projection.
		CSL_RETURN(distance(p, proj));
	});

	auto triangleDistance = define_function<Float>([&](vec2 p, vec4 tri, Float width) {
		// Point at the bottom center, shared by all triangles.
		vec2 point0 = vec2(0.5, 0.37);
		// Distance to each segment.
		Float minDist = segmentDistance(p, point0, tri[x, y]);
		minDist = min(minDist, segmentDistance(p, tri[x, y], tri[z, w]));
		minDist = min(minDist, segmentDistance(p, tri[z, w], point0));
		// Smooth result for transition.
		CSL_RETURN(1.0 - smoothstep(0.0, width, minDist));
	});

	/// Text utilities.
	auto getLetter = define_function<Float>([&](Int lid, vec2 uv) {
		// If outside, return arbitrarily high distance.
		//CSL_IF(uv[x] < 0.0 || uv[y] < 0.0 || uv[x] > 1.0 || uv[y] > 1.0) {
		//	CSL_RETURN(1000.0);
		//}
		//// The font texture is 16x16 glyphs.
		//Int vlid = lid / 16;
		//Int hlid = lid - 16 * vlid;
		//vec2 fontUV = (vec2(Float(hlid), Float(vlid)) + uv) / 16.0;
		//// Fetch in a 3x3 neighborhood to box blur
		//Float accum = 0.0;
		//CSL_FOR(Int i = -1; i < 2; ++i) {
		//	CSL_FOR(Int j = -1; j < 2; ++j) {
		//		vec2 offset = vec2(i, j) / 1024.0;
		//		accum += texture(iChannel0, fontUV + offset, 0.0)[a];
		//	}
		//}
		//CSL_RETURN(accum / 9.0);
	});

	return shader;
}

std::unordered_map<std::string, ShaderExample> get_all_suite()
{
	std::unordered_map<std::string, ShaderExample> suite;

	{
		ShaderExample test_shader;
		test_shader.init_shader(test_frag_ops);
		suite.emplace("test", std::move(test_shader));
	}

	{
		ShaderExample test_vertex;
		test_vertex.init_shader(test_vert_quad);
		suite.emplace("test_vertex", std::move(test_vertex));
	}

	{
		ShaderExample test_frag;
		test_frag.init_shader(test_frag_quad);
		suite.emplace("test_frag", std::move(test_frag));
	}

	{
		ShaderExample frag_80;
		frag_80.init_shader(test_80);
		suite.emplace("frag_80", std::move(frag_80));
	}

	return suite;
}

struct LoopData {
	std::unordered_map<std::string, ShaderExample> shader_suite;
	GLTexture tex;
	GLProgram program;
	GLFramebuffer fb;
};

void main_loop(LoopData& data)
{
	using namespace v2;

	enum class Mode {
		Debug, ImGui, GLSL, Timings
	};

	static const std::unordered_map<Mode, std::string> mode_strs = {
		{ Mode::Debug, "Debug"},
		{ Mode::ImGui, "ImGui"},
		{ Mode::GLSL, "GLSL"},
		{ Mode::Timings, "Timings"}
	};

	static bool first = true;
	if (first) {
		std::filesystem::path path = "../resources/shadertoy-font-25.png";
		data.tex.load(std::filesystem::absolute(path).string());

		data.fb.init(512, 512);
		data.program.init_from_source(
			data.shader_suite.find("test_vertex")->second.m_glsl_str,
			data.shader_suite.find("test_frag")->second.m_glsl_str
		);

		first = false;
	}
	if (ImGui::Begin("test texture")) {

		static float time = 0.0f;

		GLvao vao;
		data.fb.clear();
		data.fb.bind();
		glViewport(0, 0, static_cast<GLsizei>(ImGui::GetContentRegionAvail().x), static_cast<GLsizei>(ImGui::GetContentRegionAvail().y));
		data.program.use();
		data.program.set_uniform("time", glUniform1f, time);
		vao.bind();
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		time += ImGui::GetIO().DeltaTime;

		ImGui::Image((ImTextureID)(static_cast<std::size_t>(data.fb.m_color.m_gl)), ImVec2(512, 512));
	}
	ImGui::End();

	static auto current_shader = data.shader_suite.begin();

	if (ImGui::Begin("Test shader")) {
		for (auto it = data.shader_suite.begin(); it != data.shader_suite.end(); ++it) {

			ImGui::SameLine();
			const bool active = it == current_shader;
			if (active) {
				ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.7f, 0.7f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.8f, 0.8f));
			}
			if (ImGui::Button(it->first.c_str())) {
				current_shader = it;
			}
			if (active) {
				ImGui::PopStyleColor(3);
			}
		}
		for (auto it = data.shader_suite.begin(); it != data.shader_suite.end(); ++it) {
			if (current_shader != it) {
				continue;
			}
			const auto& shader = *it;
			if (ImGui::BeginTabBar("mode_bar")) {
				for (const auto& mode : mode_strs) {
					if (ImGui::BeginTabItem(mode.second.c_str())) {

						switch (mode.first)
						{
						case Mode::Debug:
							ImGui::TextWrapped(shader.second.m_debug_str.c_str());
							break;
						case Mode::ImGui:
						{
							ImGuiData data;
							shader.second.m_controller.print_imgui(data);
							break;
						}
						case Mode::GLSL:
						{
							ImGui::TextWrapped(shader.second.m_glsl_str.c_str());
							break;
						}
						case Mode::Timings:
						{
							std::stringstream s;
							s << "shader traversal : " << shader.second.m_generation_timing << " ms\n";
							s << "debug generation : " << shader.second.m_debug_timing << " ms\n";
							s << "glsl generation : " << shader.second.m_glsl_timing << " ms\n";
							ImGui::TextWrapped(s.str().c_str());
							break;
						}
						break;
						default:
							break;
						}

						ImGui::EndTabItem();
					}
				}
				ImGui::EndTabBar();
			}
		}
	}
	ImGui::End();
}

int main()
{
	//for (int i = 0; i < 100000; ++i) {
	//	get_all_suite();
	//}
	//return 0;


	test_old();

	LoopData data;
	data.shader_suite = get_all_suite();
	create_context_and_run([&] {
		main_loop(data);
	});

	//test_accessor();
	testsCompliance();
	//testArgCleaning();
	//testStructsMacros();
	//testArgsOrder();
	//testInArgs();

	auto start = std::chrono::steady_clock::now();

	// readme examples
	std::string operators_str = operators_example();
	std::string arrays_str = arrays_example();
	std::string swizzling_str = swizzling_example();
	std::string auto_naming_str = auto_naming_example();
	std::string qualifier_str = qualifier_example();
	std::string functions_str = functions_example();
	std::string structure_str = structure_stratements_example();
	std::string structs_str = structs_examples();
	std::string interface_str = interface_examples();
	std::string struct_interface_comma_str = struct_interface_comma_examples();
	std::string shader_stage_str = shader_stage_options();
	std::string variations_str = meta_variations();

	// Rendu
	std::string blur_str = blurShader();
	std::string ambiant_str = ambiantShader();
	std::string ssao_str = ssaoShader();

	// Shading Language Cookbook
	std::string discard_str = discardFrag();
	std::string transfeedBack_str = transfeedBackVertex();

	// Shadertoy example
	std::string eightiesShader_str = eightiesShader();

	// Dolphin ubershaders
	std::string dolphin_vertex_str = dolphinVertex();

	auto dolphin_frag_start = std::chrono::steady_clock::now();
	std::string dolphin_frag_str = dolphinFragment();
	auto dolphin_frag_duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - dolphin_frag_start);

	//basic shaders
	auto phong_frag_start = std::chrono::steady_clock::now();
	std::string phongShading_str = phongShading();
	auto phong_frag_duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - phong_frag_start);

	std::string per_tri_normal_str = per_triangle_normal_geom();
	std::string tesselation_control_str = tesselation_control_example();
	std::string tesselation_evaluation_str = tesselation_evaluation_example();
	std::string tesselation_interfaces_str = tesselation_interfaces();

	//only measuring generation, not display
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start);

	//display

	auto seperator = [] {
		std::cout << std::string(75, '/') << "\n\n";
	};

	for (std::string str :
	{ operators_str, arrays_str, swizzling_str, auto_naming_str, qualifier_str,
		functions_str, structure_str, structs_str, interface_str,
		struct_interface_comma_str, shader_stage_str, variations_str
	})
	{
		std::cout << str;
		seperator();
	}

	for (std::string str :
	{ blur_str, ambiant_str, ssao_str, discard_str,
		transfeedBack_str, eightiesShader_str, phongShading_str, per_tri_normal_str,
		tesselation_control_str, tesselation_evaluation_str, tesselation_interfaces_str,
		dolphin_vertex_str, dolphin_frag_str
	})
	{
		std::cout << str;
		seperator();
	}

	std::cout << "all shaders total generation elapsed time : " << duration.count() / 1000.0 << " ms" << std::endl;
	std::cout << "phong fragment shader generation elapsed time : " << phong_frag_duration.count() / 1000.0 << " ms" << std::endl;
	std::cout << "dolphin fragment shader generation elapsed time : " << dolphin_frag_duration.count() / 1000.0 << " ms" << std::endl;


	// for profiling
	//for (std::size_t i = 0; i < 100; ++i) {
	//	dolphinVertex();
	//	dolphinFragment();
	//}

	return 0;
}
