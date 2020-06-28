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
	Qualify<vec2, Uniform> iResolution("iResolution");

	Qualify<Float, Uniform> iTime("iTime");
	Qualify<vec4, Out> fragColor("fragColor");

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
		CSL_IF(uv[x] < 0.0 || uv[y] < 0.0 || uv[x] > 1.0 || uv[y] > 1.0) {
			CSL_RETURN(1000.0);
		}
		// The font texture is 16x16 glyphs.
		Int vlid = lid / 16;
		Int hlid = lid - 16 * vlid;
		vec2 fontUV = (vec2(hlid, vlid) + uv) / 16.0;
		// Fetch in a 3x3 neighborhood to box blur
		Float accum = 0.0;
		CSL_FOR(Int i = -1; i < 2; ++i) {
			CSL_FOR(Int j = -1; j < 2; ++j) {
				vec2 offset = vec2(i, j) / 1024.0;
				vec2 uv_tex = fontUV + offset;
				uv_tex[y] = 1.0 - uv_tex[y];
				accum += texture(iChannel0, uv_tex, 0.0)[a];
			}
		}
		CSL_RETURN(accum / 9.0);
	});

	auto textGradient = define_function<vec3>([](Float interior, Float top, vec2 alphas) {
		// Use squared blend for the interior gradients.
		vec2 alphas2 = alphas * alphas;
		// Generate the four possible gradients (interior/edge x upper/lower)
		vec3 bottomInterior = mix(vec3(0.987, 0.746, 0.993), vec3(0.033, 0.011, 0.057), alphas2[x]);
		vec3 bottomExterior = mix(vec3(0.633, 0.145, 0.693), vec3(0.977, 1.000, 1.000), alphas[x]);
		vec3 topInterior = mix(vec3(0.024, 0.811, 0.924), vec3(0.600, 0.960, 1.080), alphas2[y]);
		vec3 topExterior = mix(vec3(0.494, 0.828, 0.977), vec3(0.968, 0.987, 0.999), alphas[y]);
		// Blend based on current location.
		vec3 gradInterior = mix(bottomInterior, topInterior, top);
		vec3 gradExterior = mix(bottomExterior, topExterior, top);
		CSL_RETURN(mix(gradExterior, gradInterior, interior));
	});

	shader.main(
		[&] {
		// Normalized pixel coordinates.
		vec2 uv = gl_FragCoord[x, y] / iResolution[x, y];
		uv[y] = 1.0 - uv[y];
		vec2 uvCenter = 2.0 * uv - 1.0;

		/// Background.
		// Color gradient.
		vec3 finalColor = 1.5 * mix(vec3(0.308, 0.066, 0.327), vec3(0.131, 0.204, 0.458), uv[x]);

		const Float gridHeight = 0.3;
		CSL_IF(uv[y] < gridHeight) {

			/// Bottom grid.
			// Compute local cflipped oordinates for the grid.
			vec2 localUV = uv * vec2(2.0, -1.0 / gridHeight) + vec2(-1.0, 1.0);
			// Perspective division, scaling, foreshortening and alignment.
			localUV[x] = localUV[x] / (localUV[y] + 0.8);
			localUV *= vec2(10.0, 20.0);
			localUV[y] = sqrt(localUV[y]);
			localUV[x] += 0.5;
			// Generate grid smooth lines (translate along time).
			vec2 unitUV = fract(localUV - vec2(0.0, 0.3 * iTime));
			vec2 gridAxes = smoothstep(0.02, 0.07, unitUV) * (1.0 - smoothstep(0.93, 0.98, unitUV));
			Float gridAlpha = 1.0 - clamp(gridAxes[x] * gridAxes[y], 0.0, 1.0);

			/// Fixed star halos.
			// Loop UVs.
			vec2 cyclicUV = mod(localUV - vec2(0.0, 0.3 * iTime), vec2(9.0, 5.0));
			// Distance to some fixed grid vertices.
			const Float haloTh = 0.6;
			Float isBright1 = 1.0 - min(distance(cyclicUV, vec2(6.0, 3.0)), haloTh) / haloTh;
			Float isBright2 = 1.0 - min(distance(cyclicUV, vec2(1.0, 2.0)), haloTh) / haloTh;
			Float isBright3 = 1.0 - min(distance(cyclicUV, vec2(3.0, 4.0)), haloTh) / haloTh;
			Float isBright4 = 1.0 - min(distance(cyclicUV, vec2(2.0, 1.0)), haloTh) / haloTh;
			// Halos brightness.
			Float spotLight = isBright1 + isBright2 + isBright3 + isBright4;
			spotLight *= spotLight;
			// Composite grid lines and halos.
			finalColor += 0.15 * gridAlpha * (1.0 + 5.0 * spotLight);

		} CSL_ELSE{
			/// Starfield.
			// Compensate aspect ratio for circular stars.
			vec2 ratioUVs = uv * vec2(1.0, iResolution[y] / iResolution[x]);
		// Decrease density towards the bottom of the screen.
		Float baseDens = clamp(uv[y] - 0.3, 0.0, 1.0);
		// Three layers of stars with varying density, cyclic animation.
		Float deltaDens = 20.0 * (sin(0.05 * iTime - 1.5) + 1.0);
		finalColor += 0.50 * stars(ratioUVs, 0.10 * baseDens, 150.0 - deltaDens);
		finalColor += 0.75 * stars(ratioUVs, 0.05 * baseDens,  80.0 - deltaDens);
		finalColor += 1.00 * stars(ratioUVs, 0.01 * baseDens,  30.0 - deltaDens);
		}

			/// Triangles.
			// Triangles upper points.
		vec4 points1 = vec4(0.30, 0.85, 0.70, 0.85);
		vec4 points2 = vec4(0.33, 0.83, 0.73, 0.88);
		vec4 points3 = vec4(0.35, 0.80, 0.66, 0.82);
		vec4 points4 = vec4(0.38, 0.91, 0.66, 0.87);
		vec4 points5 = vec4(0.31, 0.89, 0.72, 0.83);
		// Randomly perturb based on time.
		points2 += 0.04 * noise(10.0 * points2 + 0.4 * iTime);
		points3 += 0.04 * noise(10.0 * points3 + 0.4 * iTime);
		points4 += 0.04 * noise(10.0 * points4 + 0.4 * iTime);
		points5 += 0.04 * noise(10.0 * points5 + 0.4 * iTime);
		// Intensity of the triangle edges.
		Float tri1 = triangleDistance(uv, points1, 0.010);
		Float tri2 = triangleDistance(uv, points2, 0.005);
		Float tri3 = triangleDistance(uv, points3, 0.030);
		Float tri4 = triangleDistance(uv, points4, 0.005);
		Float tri5 = triangleDistance(uv, points5, 0.003);
		Float intensityTri = 0.9 * tri1 + 0.5 * tri2 + 0.2 * tri3 + 0.6 * tri4 + 0.5 * tri5;
		// Triangles color gradient, from left to right.
		Float alphaTriangles = clamp((uv[x] - 0.3) / 0.4, 0.0, 1.0);
		vec3 baseTriColor = mix(vec3(0.957, 0.440, 0.883), vec3(0.473, 0.548, 0.919), alphaTriangles);
		// Additive blending.
		finalColor += intensityTri * baseTriColor;

		/// Horizon gradient.
		const Float horizonHeight = 0.025;
		Float horizonIntensity = 1.0 - min(abs(uv[y] - gridHeight), horizonHeight) / horizonHeight;
		// Modulate base on distance to screen edges.
		horizonIntensity *= (1.0 - 0.7 * abs(uvCenter[x]) + 0.5);
		finalColor += 2.0 * horizonIntensity * baseTriColor;

		/// Letters.
		// Centered UVs for text box.
		vec2 textUV = uvCenter * 2.2 - vec2(0.0, 0.5);
		CSL_IF(abs(textUV[x]) < 1.0 && abs(textUV[y]) < 1.0) {
			// Rescale UVs.
			textUV = textUV * 0.5 + 0.5;
			textUV[x] *= 3.5;
			// Per-sign UV, manual shifts for kerning.
			const vec2 letterScaling = vec2(0.47, 0.93);
			vec2 uvLetter1 = (textUV - vec2(0.60, 0.50)) * letterScaling + 0.5;
			vec2 uvLetter2 = (textUV - vec2(1.68, 0.50)) * letterScaling + 0.5;
			vec2 uvLetter3 = (textUV - vec2(2.70, 0.50)) * letterScaling + 0.5;
			// Get letters distance to edge, merge.
			Float let1 = getLetter(179, uvLetter1);
			Float let2 = getLetter(163, uvLetter2);
			Float let3 = getLetter(188, uvLetter3);
			// Merge and threshold.
			Float finalDist = 0.52 - min(let1, min(let2, let3));
			// Split between top and bottom gradients (landscape in the reflection).
			Float localTh = 0.49 + 0.03 * noise(70.0 * uv[x] + iTime);
			Float isTop = smoothstep(localTh - 0.01, localTh + 0.01, textUV[y]);
			// Split between interior and edge gradients.
			Float isInt = smoothstep(0.018, 0.022, finalDist);
			// Compute relative heights along the color gradients (both bottom and up (shifted)), rescale.
			vec2 localUBYs = vec2(1.8 * (textUV[y] - 0.5) + 0.5);
			localUBYs[y] -= isTop * 0.5;
			vec2 gradientBlend = localUBYs / localTh;
			// Evaluate final mixed color gradient.
			vec3 textColor = textGradient(isInt, isTop, gradientBlend);
			// Add sharp reflection along a flat diagonal.
			CSL_IF(textUV[x] - 20.0 * textUV[y] < -14.0 || textUV[x] - 20.0 * textUV[y] > -2.5) {
				textColor += 0.1;
			}
			// Soft letter edges.
			Float finalDistSmooth = smoothstep(-0.0025, 0.0025, finalDist);
			finalColor = mix(finalColor, textColor, finalDistSmooth);
		}

		/// Vignetting.
		const Float radiusMin = 0.8;
		const Float radiusMax = 1.8;
		Float vignetteIntensity = (length(uvCenter) - radiusMin) / (radiusMax - radiusMin);
		finalColor *= clamp(1.0 - vignetteIntensity, 0.0, 1.0);

		/// Exposure tweak, output.
		fragColor = vec4(pow(finalColor, vec3(1.2)), 1.0);
	});

	return shader;
}

auto test_tex()
{
	using namespace v2::glsl::frag_420;
	using namespace v2::swizzles::all;
	Shader shader;

	Qualify<sampler2D, Layout<Binding<0>>, Uniform> iChannel0("iChannel0");
	Qualify<vec4, Out> fragColor("fragColor");
	Qualify<vec2, In, Layout<Location<0>>> uv("uv");

	shader.main([&] {
		fragColor = texture(iChannel0, uv);
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

	{
		ShaderExample tex_frag;
		tex_frag.init_shader(test_tex);
		suite.emplace("tex_frag", std::move(tex_frag));
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

	enum class Mode : std::size_t {
		Debug = 2, ImGui = 1, GLSL = 0, Timings = 3
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

		data.program.init_from_source(
			data.shader_suite.find("test_vertex")->second.m_glsl_str,
			data.shader_suite.find("frag_80")->second.m_glsl_str
		);

		first = false;
	}

	static auto current_shader = data.shader_suite.begin();

	if (ImGui::Begin("Shader suite")) {

		const float w = ImGui::GetContentRegionAvail().x;
		const float h = ImGui::GetContentRegionAvail().y;

		ImGui::BeginChild("left pane", ImVec2(w / 4, 0), true);
		for (auto it = data.shader_suite.begin(); it != data.shader_suite.end(); ++it) {
			if (ImGui::Selectable(it->first.c_str(), it == current_shader)) {
				current_shader = it;
			}
		}
		ImGui::EndChild();
		ImGui::SameLine();
		ImGui::BeginGroup();
		const auto& shader = *current_shader;
		if (ImGui::BeginTabBar("mode_bar", ImGuiTabBarFlags_Reorderable)) {
			for (const auto& mode : mode_strs) {
				if (ImGui::BeginTabItem(mode.second.c_str())) {
					ImGui::BeginChild("text");
					switch (mode.first)
					{
					case Mode::Debug:
					{
						const auto& str = shader.second.m_debug_str;
						ImGui::TextUnformatted(str.data(), str.data() + str.size());
						break;
					}
					case Mode::ImGui:
					{
						ImGuiData data;
						shader.second.m_controller.print_imgui(data);
						break;
					}
					case Mode::GLSL:
					{
						const auto& str = shader.second.m_glsl_str;
						ImGui::TextUnformatted(str.data(), str.data() + str.size());
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
					ImGui::EndChild();
					ImGui::EndTabItem();
				}
			}
			ImGui::EndTabBar();
		}
		ImGui::EndGroup();
	}
	ImGui::End();

	if (ImGui::Begin("OpenGL rendering")) {

		static float time = 0.0f;

		const float w = ImGui::GetContentRegionAvail().x;
		const float h = ImGui::GetContentRegionAvail().y;

		data.fb.resize(static_cast<int>(w), static_cast<int>(h));

		GLvao vao;
		data.fb.clear();
		glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));
		data.program.use();
		data.tex.bind_slot(GL_TEXTURE0);
		data.program.set_uniform("iTime", glUniform1f, time);
		data.program.set_uniform("iResolution", glUniform2f, w, h);
		vao.bind();
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		time += ImGui::GetIO().DeltaTime;

		ImGui::Image((ImTextureID)(static_cast<std::size_t>(data.fb.m_color.m_gl)), ImVec2(w, h));
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

	std::cout << data.shader_suite.find("frag_80")->second.m_glsl_str << std::endl;

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
