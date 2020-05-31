#include <shaders/80_s_shader.h>
#include <shaders/dolphin.h>
#include <shaders/rendu_compilation.h>
#include <shaders/ogl4_sl_cookbook.h>
#include <shaders/examples.h>

#include "tests.h"

#include "v2/Listeners.hpp"

#include <iostream>
#include <chrono>

void testv2()
{
	using namespace v2;

	auto shader = std::make_shared<ShaderController>();
	listen().current_shader = shader;

	{
		using namespace v2::swizzles::rgba;

		Qualify<Float, Layout<Binding<0>>> f;
		vec3 v;
		Qualify<vec3, Uniform, Array<5, 7>> av;
		
		using AA = decltype(av);
		using A = typename AA::ArrayComponent;

		mat3 m;

		Qualify<float, Uniform> ff;

		{
			auto vf = v * f;
			auto fv = f * v;
			auto ff = f * f;
			auto vv = v * v;
			auto mf = m * f;
			auto mv = m * v;
			auto mm = m * m;
		}

		{
			auto vf = v + f;
			auto fv = f + v;
			auto ff = f + f;
			auto vv = v + v;
			auto mf = m + f;
			auto mm = m + m;
		}

		{
			auto formula = f * (-m[0] * v[b, g, a] - m[1] * m[2]);
		}

		mat3 mu = mat3(vec3(0.0, f, v[g]), v, av[2][3]);

		auto fuu = define_function<vec3, Float>(
			[](mat3 m) {
			CSL_WHILE(true) {
				m + m;
				CSL_BREAK;
			}
			CSL_FOR(Int w; false; ) {
				w + w;
				CSL_CONTINUE;
			}
		}, [](vec3 v) {
			CSL_IF(true) {
				v = v;
			} CSL_ELSE_IF(false) {
				CSL_IF(false) {
					v - v;
				}
				v* v;
			} CSL_ELSE{
				v + v;
			}
		});

		int i = 0;
		CSL_SWITCH(i) {
			CSL_CASE(0) : { Int j; }
		CSL_DEFAULT: { vec3 kk; }
		}

		auto fun = define_function<void>([]() {});
		fun();

		fuu(m) + fuu(m * m);
	}

	std::cout << std::endl;

	DebugData data;
	shader->print_debug(data);
	std::cout << data.stream.str() << std::endl;
}

int main()
{
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

	testv2();

	return 0;
}
