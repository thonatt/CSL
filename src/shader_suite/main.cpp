#include <shaders/80_s_shader.h>
#include <shaders/dolphin.h>
#include <shaders/rendu_compilation.h>
#include <shaders/ogl4_sl_cookbook.h>
#include <shaders/examples.h>

#include "tests.h"

#include "v2/Listeners.hpp"
#include "v2/Structs.hpp"

#include <iostream>
#include <chrono>
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>

#include "v2/ToDebug.hpp"
#include "v2/glsl/ToGLSL.hpp"
#include "ToImGui.hpp"

void test_shader_body(v2::ShaderController& shader, std::string& str)
{
	using namespace v2;

	static bool first = true;

	if (first)
	{
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
				b.plop.v = b.plop.v;
			}
		});

		using T = vec3;

		Qualify<T, Uniform, Array<3, 2>> b;

		auto m = define_function<void>("main", [&]
		{
			using namespace v2::swizzles::xyzw;
			b[2][1][x, y, z][z, z, y];
			ffff(b[2][1], b[2][1]) + ffff(b[2][1], b[2][1]);
		//T pp;
		//pp = b[2];
		});

		DebugData data;
		shader.print_debug(data);
		str = data.stream.str();

		first = false;
	}

}


void main_loop()
{
	using namespace v2;

	static auto shader = std::make_shared<ShaderController>();
	listen().current_shader = shader;

	static std::string debug_str;
	test_shader_body(*shader, debug_str);

	enum class Mode {
		Debug, ImGui, GLSL
	};

	static const std::unordered_map<Mode, std::string> mode_strs = {
		{ Mode::Debug, "Debug"},
		{ Mode::ImGui, "ImGui"},
		{ Mode::GLSL, "GLSL"}
	};

	if (ImGui::Begin("Test shader")) {
		if (ImGui::BeginTabBar("mode_bar")) {
			for (const auto& mode : mode_strs) {
				if (ImGui::BeginTabItem(mode.second.c_str())) {

					switch (mode.first)
					{
					case Mode::Debug:
						ImGui::TextWrapped(debug_str.c_str());
						break;
					case Mode::ImGui:
					{
						ImGuiData imgui_data;
						shader->print_imgui(imgui_data);
						break;
					}
					case Mode::GLSL:
					{
						GLSLData glsl_data;
						shader->print_glsl(glsl_data);
						ImGui::TextWrapped(glsl_data.stream.str().c_str());
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
	ImGui::End();
}

void create_context()
{

	if (!glfwInit()) {
		std::cout << "glfwInit Initialization failed " << std::endl;
		return;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	if (!mode) {
		return;
	}
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

	//window = glfwCreateWindow(mode->width, mode->height, name.c_str(), glfwGetPrimaryMonitor(), NULL);
	auto window = std::shared_ptr<GLFWwindow>(glfwCreateWindow(1600, 1000, "bouh", NULL, NULL), glfwDestroyWindow);

	if (!window) {
		std::cout << " Window or OpenGL context creation failed " << std::endl;
	}

	glfwMakeContextCurrent(window.get());
	const int desired_fps = 60;
	const int interval = mode->refreshRate / desired_fps;
	glfwSwapInterval(interval);

	glfwSetMouseButtonCallback(window.get(), ImGui_ImplGlfw_MouseButtonCallback);
	glfwSetKeyCallback(window.get(), ImGui_ImplGlfw_KeyCallback);
	glfwSetCharCallback(window.get(), ImGui_ImplGlfw_CharCallback);
	glfwSetScrollCallback(window.get(), ImGui_ImplGlfw_ScrollCallback);

	const int version = gladLoadGL();
	std::cout << "OpenGL version " << GLVersion.major << "." << GLVersion.minor << std::endl;

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui::StyleColorsDark();

	//imgui scaling
	float xscale, yscale;
	glfwGetMonitorContentScale(glfwGetPrimaryMonitor(), &xscale, &yscale);

	const float scaling = std::max(xscale, yscale);
	ImGui::GetStyle().ScaleAllSizes(scaling);
	ImGui::GetIO().FontGlobalScale = scaling;

	ImGui_ImplGlfw_InitForOpenGL(window.get(), false);
	ImGui_ImplOpenGL3_Init("#version 410");

	while (!glfwWindowShouldClose(window.get())) {

		glClear((GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGuiIO& io = ImGui::GetIO();
		if (io.KeysDown[GLFW_KEY_ESCAPE]) {
			glfwSetWindowShouldClose(window.get(), GLFW_TRUE);
		}

		if (io.KeysDown[GLFW_KEY_LEFT_CONTROL] && io.MouseWheel != 0) {
			static float scale = scaling;
			scale *= std::pow(1.05f, io.MouseWheel);

			ImGuiStyle style;
			style.ScaleAllSizes(scale);
			ImGui::GetStyle() = style;
			ImGui::GetIO().FontGlobalScale = scale;
		}

		main_loop();

		static bool show_imgui = false;
		if (io.KeysDownDuration[GLFW_KEY_H] == 0) {
			show_imgui = !show_imgui;
		}
		if (show_imgui) {
			ImGui::ShowDemoWindow();
		}

		glClearColor(0, 0, 0, 0);
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window.get());
	}
}

//struct V1 { };
//struct V2 { };
//
//struct A : virtual v2::VisitableBase<V1, V2> { };
//struct B : virtual A, v2::VisitableDerived<B, V1, V2> { };
//struct C : virtual A, v2::VisitableDerived<C, V1, V2> { };

void tt()
{
	//using namespace v2;

	//V1 v1;
	//V2 v2;
	//
	//auto b = std::static_pointer_cast<A>(std::make_shared<B>());
	//auto c = std::static_pointer_cast<A>(std::make_shared<C>());

	//b->visit(v1);
	//b->visit(v2);
	//c->visit(v1);
	//c->visit(v2);
}
//
//void testv2()
//{
//	using namespace v2;
//
//	auto shader = std::make_shared<ShaderController>();
//	listen().current_shader = shader;
//
//	CSL2_STRUCT(Plop,
//		(vec3)v,
//		(Float)f
//	);
//
//	CSL2_STRUCT(BigPlop,
//		(Plop)plop,
//		(Float)g
//	);
//
//	auto ffff = define_function<void>([&] {
//		BigPlop b;
//		b.plop.v* BigPlop().plop.f;
//	});
//
//	Qualify<Plop, Uniform, Array<3>> b;
//
//	{
//		Plop pp;
//		pp = b[2];
//	}
//
//
//	//Qualify<Plop, > plops;
//
//
//	{
//		using namespace v2::swizzles::rgba;
//
//		//Qualify<Float, Layout<Binding<0>>> f;
//		//vec3 v;
//		//Qualify<vec3, Uniform, Array<5, 7>> av;
//
//		//using AA = decltype(av);
//		//using A = typename AA::ArrayComponent;
//
//		//mat3 m;
//
//		//listen().add_struct<TestStruct>();
//
//		//Qualify<float, Uniform> ff;
//
//		//{
//		//	auto vf = v * f;
//		//	auto fv = f * v;
//		//	auto ff = f * f;
//		//	auto vv = v * v;
//		//	auto mf = m * f;
//		//	auto mv = m * v;
//		//	auto mm = m * m;
//		//}
//
//		//{
//		//	auto vf = v + f;
//		//	auto fv = f + v;
//		//	auto ff = f + f;
//		//	auto vv = v + v;
//		//	auto mf = m + f;
//		//	auto mm = m + m;
//		//}
//
//		//{
//		//	auto formula = f * (-m[0] * v[b, g, a] - m[1] * m[2]);
//		//}
//
//		//mat3 mu = mat3(vec3(0.0, f, v[g]), v, av[2][3]);
//
//		//auto fuu = define_function<vec3, Float>(
//		//	[](mat3 m) {
//		//	CSL_WHILE(true) {
//		//		m + m;
//		//		CSL_BREAK;
//		//	}
//		//	CSL_FOR(Int w; false; ) {
//		//		w + w;
//		//		CSL_CONTINUE;
//		//	}
//		//}, [](vec3 v) {
//		//	CSL_IF(true) {
//		//		v = v;
//		//	} CSL_ELSE_IF(false) {
//		//		CSL_IF(false) {
//		//			v - v;
//		//		}
//		//		v* v;
//		//	} CSL_ELSE{
//		//		v + v;
//		//	}
//		//});
//
//		//int i = 0;
//		//CSL_SWITCH(i) {
//		//	CSL_CASE(0) : { Int j; }
//		//CSL_DEFAULT: { vec3 kk; }
//		//}
//
//		//auto fun = define_function<void>([]() {});
//		//fun();
//
//		//fuu(m) + fuu(m * m);
//
//	}
//
//	std::cout << std::endl;
//
//	DebugData data;
//	shader->print_debug(data);
//	std::cout << data.stream.str() << std::endl;
//}

int main()
{

	create_context();

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
