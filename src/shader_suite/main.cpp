#include <shaders/80_s_shader.h>
#include <shaders/dolphin.h>
#include <shaders/rendu_compilation.h>
#include <shaders/ogl4_sl_cookbook.h>
#include <shaders/examples.h>

#include "tests.hpp"

#include "v2/Listeners.hpp"
#include "v2/Structs.hpp"

#include <array>
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

	ShaderExample() = default;

	template<typename ShaderCreation>
	ShaderExample(ShaderCreation&& f)
	{
		using namespace v2;

		auto start = Clock::now();
		auto shader = f();
		m_generation_timing = std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - start).count() / 1000.0;

		m_debug_timing = get_timing([&]
		{
			DebugData debug_data;
			shader.print_debug<Dummy>(debug_data);
			m_debug_str = debug_data.stream.str();
		});

		m_glsl_timing = get_timing([&]
		{
			GLSLData glsl_data;
			shader.print_glsl(glsl_data);
			m_glsl_str = glsl_data.stream.str();
		});

		std::swap(m_controller, shader.get_base());
	}

	GLProgram m_program;
	std::string m_debug_str, m_glsl_str;
	v2::ShaderController m_controller;
	std::string m_name;
	double m_generation_timing, m_glsl_timing, m_debug_timing;
};

using ShaderPtr = std::shared_ptr<ShaderExample>;

v2::glsl::frag_420::Shader test_frag_ops()
{
	using namespace v2::glsl::frag_420;
	Shader shader;

	CSL2_STRUCT(Plop,
		(vec3, v),
		(Float, f)
	);

	CSL2_STRUCT(BigPlop,
		(Plop, plop),
		(Float, g)
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

auto vert_basic()
{
	using namespace v2::glsl::vert_420;
	using namespace v2::swizzles::xyzw;
	Shader shader;

	Qualify<vec3, Layout<Location<0>>, In> pos("pos");
	Qualify<vec2, Layout<Location<1>>, In> in_uv("in_uv");
	Qualify<vec2, Out> uv("uv");

	shader.main([&]
	{
		gl_Position = vec4(2.0 * pos - 1.0, 1.0);
		uv = in_uv;
	});

	return shader;
}

auto frag_basic()
{
	using namespace v2::glsl::frag_420;
	using namespace v2::swizzles::xyzw;
	Shader shader;

	Qualify<vec4, Layout<Location<0>>, Out> color("color");
	Qualify<vec2, In> uv("uv");
	shader.main([&]
	{
		color = vec4(mod(10.0 * uv, 1.0), 0.0, 1.0);
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

auto vertex_mvp()
{
	using namespace v2::glsl::vert_420;
	using namespace v2::swizzles::all;
	Shader shader;

	Qualify<vec3, Layout<Location<0>>, In> in_position("in_position");
	Qualify<vec2, Layout<Location<1>>, In> in_uv("in_uv");
	Qualify<vec3, Layout<Location<2>>, In> in_normal("in_normal");

	Qualify<Float, Uniform> time("time");
	Qualify<Float, Uniform> distance = Float(1.0f) << "distance";
	Qualify<Float, Uniform> n = Float(0.1f) << "near";

	Qualify<vec3, Layout<Location<0>>, Out> position("position");
	Qualify<vec3, Layout<Location<1>>, Out> normal("normal");
	Qualify<vec2, Layout<Location<2>>, Out> uv("uv");

	shader.main([&] {
		Float fov = Float(90.0) << "fov";
		Float s = 1.0 / tan(fov / (2.0 * 180.0) * 3.141519265) << "scale";
		Float f = Float(5.0) << "far";
		mat4 proj = mat4(
			vec4(s, vec3(0.0)),
			vec4(0.0, s, vec2(0.0)),
			vec4(vec2(0.0), -(f + n) / (f - n), -1.0),
			vec4(vec2(0.0), -2.0 * f * n / (f - n), 0.0)
		) << "proj";

		vec3 at = vec3(0, 0, 0) << "at";
		vec3 up = vec3(0, 0, 1) << "up";
		Float theta = mod(time / 5.0, 1.0) * 2.0 * 3.141519265 << "theta";
		vec3 eye = distance * vec3(cos(theta), sin(theta), 1.0) << "eye";

		vec3 z_axis = normalize(at - eye) << "z";
		vec3 x_axis = normalize(cross(up, z_axis)) << "x";
		vec3 y_axis = cross(z_axis, x_axis) << "y";

		mat4 view = transpose(mat4(
			vec4(x_axis, dot(x_axis, eye)),
			vec4(y_axis, dot(y_axis, eye)),
			vec4(z_axis, dot(z_axis, eye)),
			vec4(vec3(0.0), 1.0))
		) << "view";

		position = in_position;
		normal = in_normal;
		uv = in_uv;
		gl_Position = proj * view * vec4(in_position, 1.0);

	});
	return shader;
}

auto textured_mesh_frag()
{
	using namespace v2::glsl::frag_420;
	using namespace v2::swizzles::all;
	Shader shader;

	Qualify<vec2, Layout<Location<2>>, In> uv("uv");
	Qualify<sampler2D, Layout<Binding<0>>, Uniform> tex("tex");

	Qualify<vec4, Layout<Location<0>>, Out> color("color");

	shader.main([&] {
		color = texture(tex, uv);
	});
	return shader;
}

// https://www.shadertoy.com/view/XdXGW8
auto fractal_noise_frag()
{
	using namespace v2::glsl::frag_420;
	using namespace v2::swizzles::xyzw;

	Shader shader;

	Qualify<vec2, Layout<Location<0>>, In> uv("uv");
	Qualify<vec4, Layout<Location<0>>, Out> color("color");

	auto hash = define_function<vec2>([](vec2 p)
	{
		const vec2 k = vec2(0.3183099, 0.3678794);
		p = p * k + k[y, x];
		CSL_RETURN(-1.0 + 2.0 * fract(16.0 * k * fract(p[x] * p[y] * (p[x] + p[y]))));
	});

	auto noise = define_function<Float>([&](vec2 p) {
		vec2 i = floor(p);
		vec2 f = fract(p);
		vec2 u = f * f * (3.0 - 2.0 * f);
		CSL_RETURN(
			mix(mix(dot(hash(i + vec2(0.0, 0.0)), f - vec2(0.0, 0.0)),
				dot(hash(i + vec2(1.0, 0.0)), f - vec2(1.0, 0.0)), u[x]),
				mix(dot(hash(i + vec2(0.0, 1.0)), f - vec2(0.0, 1.0)),
					dot(hash(i + vec2(1.0, 1.0)), f - vec2(1.0, 1.0)), u[x]), u[y])
		);
	});

	shader.main([&] {
		const Int freq_count = 5;
		mat2 m = mat2(1.6, 1.2, -1.2, 1.6);

		Float f = 0;
		Float amplitude = 0.5;
		vec2 current_uv = 32.0 * uv;

		CSL_FOR(Int i = 0; i < freq_count; ++i) {
			f += amplitude * noise(current_uv);
			current_uv = m * current_uv;
			amplitude /= 2.0;
		}

		color = vec4(0.5 + 0.5 * vec3(f), 1.0);
	});

	return shader;
}

auto multiple_outputs_frag()
{
	using namespace v2::glsl::frag_420;
	using namespace v2::swizzles::all;
	Shader shader;

	Qualify<vec3, Layout<Location<0>>, In> position("position");
	Qualify<vec3, Layout<Location<1>>, In> normal("normal");
	Qualify<vec2, Layout<Location<2>>, In> uv("uv");

	Qualify<sampler2D, Layout<Binding<0>>, Uniform> tex("tex");

	Qualify<vec3, Layout<Location<0>>, Out> out_position("out_position");
	Qualify<vec3, Layout<Location<1>>, Out> out_normal("out_normal");
	Qualify<vec2, Layout<Location<2>>, Out> out_uv("out_uv");
	Qualify<Float, Layout<Location<3>>, Out> out_depth("out_depth");
	Qualify<vec3, Layout<Location<4>>, Out> out_tex("out_tex");

	shader.main([&] {
		out_position = 0.5 * (1.0 + position);
		out_normal = 0.5 * (1.0 + normalize(normal));
		out_uv = uv;
		out_tex = texture(tex, uv)[x, y, z];
		out_depth = gl_FragCoord[z];
	});

	return shader;
}

enum class ShaderGroup {
	Test,
	Examples,
	Shadertoy,
	Rendu,
};

struct ShaderSuite {

	template<typename F>
	void add_shader(const ShaderGroup group, const std::string& name, F&& f) {
		auto shader = std::make_shared<ShaderExample>(std::forward<F>(f));
		shader->m_name = name;
		m_shaders.emplace(name, shader);
		m_groups[group].emplace(name, shader);
	}

	std::unordered_map<std::string, ShaderPtr> m_shaders;
	std::unordered_map<ShaderGroup, std::unordered_map<std::string, ShaderPtr>> m_groups;

};

struct LoopData;

struct PipelineaBase {

	struct ShaderActive {
		ShaderPtr m_shader;
		bool m_active = true;
	};

	void add_shader(const GLenum type, const std::pair<std::string, ShaderPtr>& shader)
	{
		m_shaders[type] = { shader.second };
		m_program.set_shader_glsl(type, shader.second->m_glsl_str);
	}

	virtual void additionnal_gui() {}
	virtual void draw(LoopData& data) = 0;
	virtual ~PipelineaBase() = default;

	GLProgram m_program;
	std::unordered_map<GLenum, ShaderActive> m_shaders;
};

std::unordered_map<std::string, std::shared_ptr<PipelineaBase>> get_all_pipelines(const LoopData& data);

struct LoopData {

	void init_gl_once()
	{
		static bool first = true;
		if (!first) {
			return;
		}
		first = false;

		m_framebuffer.create_attachment(GLformat());
		m_fractal_noise.create_attachment(GLformat());

		m_pipelines = get_all_pipelines(*this);

		std::filesystem::path path = "../resources/shadertoy-font-25.png";
		m_tex_letters.load(std::filesystem::absolute(path).string());

		using v2 = std::array<float, 2>;
		using v3 = std::array<float, 3>;

		m_screen_quad.init();
		m_screen_quad.set_triangles({ 0,1,2, 0,3,2 });
		m_screen_quad.set_attributes(
			std::vector<v3>{ v3{ 0,0,0 }, v3{ 1,0,0 }, v3{ 1,1,0 }, v3{ 0,1,0 } },
			std::vector<v2>{ v2{ 0,0 }, v2{ 1,0 }, v2{ 1,1 }, v2{ 0,1 } }
		);

		m_triangle.init();
		m_triangle.set_triangles({ 0,1,2 });
		m_triangle.set_attributes(
			std::vector<v3>{ v3{ -1,-1,0 }, v3{ -1,1,0 }, v3{ 2,0,0 } },
			std::vector<v2>{ v2{ 0,0 }, v2{ 1,0 }, v2{ 1,1 } }
		);

		m_quad.init();
		m_quad.set_triangles({ 0,1,2,0,3,2 });
		m_quad.set_attributes(
			std::vector<v3>{ v3{ -1,-1,0 }, v3{ -1,1,0 }, v3{ 1,1,0 }, v3{ 1,-1,0 } },
			std::vector<v2>{ v2{ 0,0 }, v2{ 1,0 }, v2{ 1,1 }, v2{ 0,1 } }
		);

		const float t = (1.0f + std::sqrt(5.0f)) / 2.0f;
		constexpr float pi = 3.14159265f;

		m_isosphere.init();
		m_isosphere.set_triangles({ 0, 11, 5 , 0, 5, 1, 0, 1, 7,0, 7, 10 , 0, 10, 11 , 1, 5, 9, 5, 11, 4, 11, 10, 2, 10, 7, 6, 7, 1, 8,
			3, 9, 4 ,3, 4, 2,3, 2, 6, 3, 6, 8, 3, 8, 9 , 4, 9, 5 , 2, 4, 11 ,6, 2, 10,8, 6, 7, 9, 8, 1 });

		std::vector<v3> ps = {
			{-1, t, 0}, {1, t, 0}, {-1, -t, 0}, {1, -t, 0},
			{0, -1, t}, {0, 1, t}, {0, -1, -t}, {0, 1, -t},
			{t, 0, -1}, {t, 0, 1}, {-t, 0, -1}, {-t, 0, 1}
		};

		std::vector<v3> ns(ps.size());
		std::vector<v2> uvs(ps.size());
		for (std::size_t i = 0; i < ps.size(); ++i) {
			float norm = 0;
			for (int j = 0; j < 3; ++j) {
				norm += ps[i][j] * ps[i][j];
			}
			norm = 1.0f / std::sqrt(norm);
			for (int j = 0; j < 3; ++j) {
				ns[i][j] = ps[i][j] * norm;
			}
			uvs[i][0] = std::acos(ns[i][2]) / pi;
			uvs[i][1] = 0.5f * (1.0f + std::atan2(ns[i][1], ns[i][0]) / pi);
		}
		m_isosphere.set_attributes(ps, uvs, ns);

		m_current_pipeline = m_pipelines.begin()->second;
	}

	ShaderSuite m_shader_suite;
	std::unordered_map<std::string, std::shared_ptr<PipelineaBase>> m_pipelines;

	std::shared_ptr<PipelineaBase> m_current_pipeline = {};
	float m_time = 0.0f;
	GLTexture m_tex_letters, m_previous_rendering;
	GLFramebuffer m_framebuffer, m_fractal_noise;
	GLmesh m_isosphere, m_quad, m_screen_quad, m_triangle;
	int m_w_screen, m_h_screen;
};

struct PipelineGrid final : PipelineaBase {
	void draw(LoopData& data) override {
		m_program.set_uniform("time", glUniform1f, data.m_time);
		glDisable(GL_DEPTH_TEST);
		data.m_screen_quad.draw();
	}
};

struct Pipeline80 final : PipelineaBase {
	void draw(LoopData& data) override {
		m_program.set_uniform("iTime", glUniform1f, data.m_time);
		m_program.set_uniform("iResolution", glUniform2f, (float)data.m_framebuffer.m_w, (float)data.m_framebuffer.m_h);
		data.m_tex_letters.bind_slot(GL_TEXTURE0);
		glDisable(GL_DEPTH_TEST);
		data.m_screen_quad.draw();
	}
};

struct PipelineTriangle final : PipelineaBase {
	void draw(LoopData& data) override {
		m_program.set_uniform("time", glUniform1f, data.m_time);
		data.m_previous_rendering.bind_slot(GL_TEXTURE0);
		data.m_quad.draw();
	}
};

struct PipelineNoise final : PipelineaBase {
	void draw(LoopData& data) override {
		data.m_fractal_noise.resize(data.m_framebuffer.m_w, data.m_framebuffer.m_h);
		data.m_fractal_noise.bind_draw();
		glDisable(GL_DEPTH_TEST);
		data.m_screen_quad.draw();
		data.m_framebuffer.blit_from(data.m_fractal_noise);
	}
};

struct PipelineGBuffer final : PipelineaBase {

	enum class Mode : GLenum {
		Position = 0,
		Normal = 1,
		UV = 2,
		Depth = 3,
		Texture = 4
	};

	PipelineGBuffer() {
		const int sample_count = 4;
		m_gbuffer.m_sample_count = sample_count;
		m_gbuffer.add_attachments(
			GLTexture(GLformat(GL_RGB32F, GL_RGB, GL_FLOAT), sample_count),
			GLTexture(GLformat(GL_RGB32F, GL_RGB, GL_FLOAT), sample_count),
			GLTexture(GLformat(GL_RG32F, GL_RG, GL_FLOAT), sample_count),
			GLTexture(GLformat(GL_R32F, GL_RED, GL_FLOAT), sample_count),
			GLTexture(GLformat(GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE), sample_count)
		);
	}

	void draw(LoopData& data) override {
		
		// render noise texture
		{
			data.m_pipelines.find("noise")->second->m_program.use();
			data.m_fractal_noise.resize(data.m_framebuffer.m_w, data.m_framebuffer.m_h);
			data.m_fractal_noise.clear();
			data.m_fractal_noise.bind_draw();
			glDisable(GL_DEPTH_TEST);
			data.m_screen_quad.draw();
			data.m_fractal_noise.m_attachments[0].bind();
			glGenerateMipmap(data.m_fractal_noise.m_attachments[0].m_target);
		}

		m_program.use();
		m_gbuffer.resize(data.m_framebuffer.m_w, data.m_framebuffer.m_h);
		m_gbuffer.clear();

		m_gbuffer.bind_draw();
		m_program.set_uniform("time", glUniform1f, data.m_time);
		m_program.set_uniform("distance", glUniform1f, 2.0f);
		m_program.set_uniform("near", glUniform1f, 0.8f);
		data.m_fractal_noise.m_attachments[0].bind_slot(GL_TEXTURE0);
		glEnable(GL_DEPTH_TEST);
		data.m_isosphere.draw();

		data.m_framebuffer.blit_from(m_gbuffer, GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(m_mode));
		if (m_mode == Mode::Depth) {
			data.m_framebuffer.m_attachments[0].set_swizzle_mask(GL_RED, GL_RED, GL_RED);
		}
	}

	void additionnal_gui() override {
		static const std::unordered_map<Mode, std::string> mode_strs = {
			{ Mode::Position, "Position" },
			{ Mode::Normal, "Normal" },
			{ Mode::UV, "UV" },
			{ Mode::Depth, "Depth" },
			{ Mode::Texture, "Texture" },
		};

		ImGui::Separator();
		ImGui::Text("Output : ");
		for (const auto& mode : mode_strs) {
			ImGui::SameLine();
			if (ImGui::RadioButton(mode.second.c_str(), mode.first == m_mode)) {
				m_mode = mode.first;
			}
		}
	}

	GLFramebuffer m_gbuffer;
	Mode m_mode = Mode::Depth;
};

ShaderSuite get_all_suite()
{
	ShaderSuite suite;

	suite.add_shader(ShaderGroup::Examples, "test", test_frag_ops);
	suite.add_shader(ShaderGroup::Test, "test_vertex", test_vert_quad);
	suite.add_shader(ShaderGroup::Test, "basic_vertex", vert_basic);

	suite.add_shader(ShaderGroup::Test, "basic_frag", frag_basic);
	suite.add_shader(ShaderGroup::Examples, "test_frag", test_frag_quad);

	suite.add_shader(ShaderGroup::Shadertoy, "frag_80", test_80);
	suite.add_shader(ShaderGroup::Test, "tex_frag", test_tex);

	suite.add_shader(ShaderGroup::Examples, "vertex_mvp", vertex_mvp);
	suite.add_shader(ShaderGroup::Examples, "textured_mesh_frag", textured_mesh_frag);

	suite.add_shader(ShaderGroup::Examples, "fractal_noise_frag", fractal_noise_frag);
	suite.add_shader(ShaderGroup::Examples, "multiple_outputs_frag", multiple_outputs_frag);
	return suite;
}

std::unordered_map<std::string, std::shared_ptr<PipelineaBase>> get_all_pipelines(const LoopData& data)
{
	std::unordered_map<std::string, std::shared_ptr<PipelineaBase>> pipelines;
	const auto& shaders = data.m_shader_suite.m_shaders;

	{
		auto pipeline = std::static_pointer_cast<PipelineaBase>(std::make_shared<Pipeline80>());
		pipeline->add_shader(GL_VERTEX_SHADER, *shaders.find("basic_vertex"));
		pipeline->add_shader(GL_FRAGMENT_SHADER, *shaders.find("frag_80"));
		pipelines.emplace("shader 80", pipeline);
	}

	{
		auto pipeline = std::static_pointer_cast<PipelineaBase>(std::make_shared<PipelineGrid>());
		pipeline->add_shader(GL_VERTEX_SHADER, *shaders.find("basic_vertex"));
		pipeline->add_shader(GL_FRAGMENT_SHADER, *shaders.find("test_frag"));
		pipelines.emplace("grid", pipeline);
	}

	{
		auto pipeline = std::static_pointer_cast<PipelineaBase>(std::make_shared<PipelineTriangle>());
		pipeline->add_shader(GL_VERTEX_SHADER, *shaders.find("vertex_mvp"));
		pipeline->add_shader(GL_FRAGMENT_SHADER, *shaders.find("textured_mesh_frag"));
		pipelines.emplace("triangle", pipeline);
	}

	{
		auto pipeline = std::static_pointer_cast<PipelineaBase>(std::make_shared<PipelineGBuffer>());
		pipeline->add_shader(GL_VERTEX_SHADER, *shaders.find("vertex_mvp"));
		pipeline->add_shader(GL_FRAGMENT_SHADER, *shaders.find("multiple_outputs_frag"));
		pipelines.emplace("gbuffer", pipeline);
	}

	{
		auto pipeline = std::static_pointer_cast<PipelineaBase>(std::make_shared<PipelineNoise>());
		pipeline->add_shader(GL_VERTEX_SHADER, *shaders.find("basic_vertex"));
		pipeline->add_shader(GL_FRAGMENT_SHADER, *shaders.find("fractal_noise_frag"));
		pipelines.emplace("noise", pipeline);
	}

	return pipelines;
}

void shader_code_gui(const std::string& code)
{
	if (code.size() < 1000) {
		ImGui::TextWrapped(code.c_str());
	} else {
		ImGui::TextUnformatted(code.data(), code.data() + code.size());
	}
}

enum class Mode : std::size_t {
	Debug = 2, ImGui = 1, GLSL = 0, Metrics = 3
};

template<typename ModeIterator>
void shader_gui(const ModeIterator& mode, ShaderExample& shader, const float vertical_size)
{
	ImGui::BeginChild(("text" + shader.m_name).c_str(), ImVec2(0, vertical_size));
	switch (mode.first)
	{
	case Mode::Debug:
	{
		shader_code_gui(shader.m_debug_str);
		break;
	}
	case Mode::ImGui:
	{
		v2::ImGuiData data;
		shader.m_controller.template print_imgui<v2::Dummy>(data);
		break;
	}
	case Mode::GLSL:
	{
		shader_code_gui(shader.m_glsl_str);
		break;
	}
	case Mode::Metrics:
	{
		std::stringstream s;
		s << "Shader traversal : " << shader.m_generation_timing << " ms\n";
		const auto& mem = shader.m_controller.m_memory_pool;
		const auto& ins = shader.m_controller.m_instruction_pool;
		s << "\t Expressions, count : " << mem.m_objects_ids.size() << ", total size : " << mem.m_buffer.size() << "\n";
		s << "\t Instructions, count : " << ins.m_objects_ids.size() << ", total size : " << ins.m_buffer.size() << "\n";
		s << "Debug generation : " << shader.m_debug_timing << " ms\n";
		s << "GLSL generation : " << shader.m_glsl_timing << " ms\n";
		ImGui::TextWrapped(s.str().c_str());
		break;
	}
	default:
		break;
	}
	ImGui::EndChild();
}

void main_loop(LoopData& data)
{
	static const std::unordered_map<Mode, std::string> mode_strs = {
		{ Mode::GLSL, "GLSL"},
		{ Mode::ImGui, "ImGui"},
		{ Mode::Debug, "Debug"},
		{ Mode::Metrics, "Metrics"}
	};

	static const std::unordered_map<ShaderGroup, std::string> shader_group_strs = {
		{ ShaderGroup::Test, "Test"},
		{ ShaderGroup::Examples, "Examples"},
		{ ShaderGroup::Shadertoy, "Shadertoy"},
		{ ShaderGroup::Rendu, "Rendu"},
	};

	static const std::unordered_map<GLenum, std::string> shader_type_strs = {
		{ GL_VERTEX_SHADER, "Vertex"},
		{ GL_TESS_CONTROL_SHADER, "TessControl"},
		{ GL_TESS_EVALUATION_SHADER, "TessEval"},
		{ GL_GEOMETRY_SHADER, "Geometry"},
		{ GL_FRAGMENT_SHADER, "Fragment"},
	};

	static ShaderPtr current_shader = {};

	data.m_time += ImGui::GetIO().DeltaTime;

	data.init_gl_once();

	// Shader suite window
	if (ImGui::Begin("Shader suite")) {

		const float w = ImGui::GetContentRegionAvail().x;
		const float h = ImGui::GetContentRegionAvail().y;

		{
			ImGui::BeginChild("left pane", ImVec2(w / 4, 0), true);
			for (const auto& group : data.m_shader_suite.m_groups)
			{
				if (group.first == ShaderGroup::Test) {
					continue;
				}
				ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
				if (ImGui::TreeNode(shader_group_strs.find(group.first)->second.c_str()))
				{
					for (const auto& shader : group.second) {
						if (ImGui::Selectable(shader.first.c_str(), shader.second == current_shader)) {
							current_shader = shader.second;

							bool pipeline_found = false;
							for (const auto& pipeline : data.m_pipelines) {
								for (const auto& shader : pipeline.second->m_shaders) {
									if (current_shader->m_name == shader.second.m_shader->m_name) {
										data.m_current_pipeline = pipeline.second;
										pipeline_found = true;
									}
								}
							}
							if (!pipeline_found) {
								data.m_current_pipeline = {};
							}
						}
					}
					ImGui::TreePop();
				}
			}
			ImGui::EndChild();
		}

		ImGui::SameLine();

		{
			ImGui::BeginChild("right pane", ImVec2(), false);

			float active_shader_count = 0;
			{
				ImGui::BeginChild("top right pane", ImVec2(0.0f, h / 10.0f), true);
				if (data.m_current_pipeline) {
					int count = 0;
					for (auto& shader : data.m_current_pipeline->m_shaders) {
						if (count > 0) {
							ImGui::SameLine();
							ImGui::Text("->");
							ImGui::SameLine();
						}
						ImGui::Checkbox((shader_type_strs.find(shader.first)->second + "##").c_str(), &shader.second.m_active);
						if (shader.second.m_active) {
							++active_shader_count;
						}
						++count;
					}
				}
				if (data.m_current_pipeline) {
					data.m_current_pipeline->additionnal_gui();
				}
				ImGui::EndChild();
			}

			{
				const float shader_code_height = (9.0f * w / 10.0f - ImGui::GetFrameHeightWithSpacing()) / active_shader_count;
				ImGui::BeginChild("bottom right pane", ImVec2(), true);
				if (ImGui::BeginTabBar("mode_bar", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs)) {
					for (const auto& mode : mode_strs) {
						if (ImGui::BeginTabItem(mode.second.c_str())) {
							if (data.m_current_pipeline) {
								int count = 0;
								for (auto& shader : data.m_current_pipeline->m_shaders) {
									if (!shader.second.m_active) {
										continue;
									}
									if (count) {
										ImGui::Separator();
									}
									shader_gui(mode, *shader.second.m_shader, shader_code_height);
									++count;
								}
							} else if (current_shader) {
								shader_gui(mode, *current_shader, shader_code_height);
							}
							ImGui::EndTabItem();
						}
					}
					ImGui::EndTabBar();
				}
				ImGui::EndChild();
			}
			ImGui::EndChild();
		}
	}
	ImGui::End();

	// OpenGL visualisation if selected shader has an associated pipeline
	if (data.m_current_pipeline) {
		if (ImGui::Begin("OpenGL rendering")) {

			const float w = ImGui::GetContentRegionAvail().x;
			const float h = ImGui::GetContentRegionAvail().y;

			data.m_framebuffer.resize(static_cast<int>(w), static_cast<int>(h));

			// default parameters, can be overrided by pipelines
			glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));
			glEnable(GL_DEPTH_TEST);
			data.m_framebuffer.m_attachments[0].set_swizzle_mask();

			data.m_framebuffer.clear(0.4f, 0.4f, 0.4f);
			data.m_framebuffer.bind_draw();
			data.m_current_pipeline->m_program.use();
			data.m_current_pipeline->draw(data);

			ImGui::Image((ImTextureID)(static_cast<std::size_t>(data.m_framebuffer.m_attachments.front().m_gl)), ImVec2(w, h));
		}
		ImGui::End();
	}
}


void test_polymorphic_vector()
{
	struct Base {
		virtual void f() = 0;
		virtual ~Base() { std::cout << "Dbase" << std::endl; }
	};

	struct DerivedA final : Base {
		~DerivedA() { std::cout << "DA" << std::endl; }
		void f() override { std::cout << "A" << std::endl; }
	};

	struct E {
		E() { std::cout << "E" << std::endl; }
		~E() { std::cout << "DE" << std::endl; }
	};

	struct DerivedB : Base, E {
		DerivedB(int j) : i(j) {}
		virtual ~DerivedB() { std::cout << "DB" << std::endl; }
		void f() override { std::cout << "B " << i << std::endl; }
		int i = 0;
	};

	struct DerivedC final : DerivedB {
		DerivedC(int j, float h) : DerivedB(j), g(h) {}
		~DerivedC() { std::cout << "DC" << std::endl; }
		void f() override { std::cout << "C " << i << " " << g << std::endl; }
		float g = 1.0f;
	};

	{
		v2::PolymorphicVector<Base, sizeof(DerivedC), alignof(DerivedC)> vec;

		vec.emplace_back<DerivedB>(12);
		vec.emplace_back<DerivedA>();
		vec.emplace_back<DerivedC>(13, 3.2f);

		for (std::size_t i = 0; i < vec.size(); ++i) {
			vec[i].f();
		}
	}

	{

		constexpr std::size_t BandSize = 16;
		v2::PolymorphicMemoryManager<Base, BandSize, sizeof(DerivedC), alignof(DerivedC)> memory;

		std::cout << std::endl;

		auto id1 = memory.emplace_back<DerivedB>(12);
		auto id2 = memory.emplace_back<DerivedA>();
		auto id3 = memory.emplace_back<DerivedC>(13, 3.2f);

		std::cout << std::endl;

		std::cout << sizeof(DerivedA) << std::endl;
		std::cout << sizeof(DerivedB) << std::endl;
		std::cout << sizeof(DerivedC) << std::endl;

		std::cout << std::endl;

		for (std::size_t i = 0; i < memory.m_buffers.size(); ++i) {
			std::cout << memory.m_buffers[i].size() / ((i + 1) * BandSize) << std::endl;
		}

		std::cout << std::endl;

		memory[id1].f();
		memory[id2].f();
		memory[id3].f();

		std::cout << std::endl;
	}


}

int main()
{
	//for profiling
	//for (int i = 0; i < 100000; ++i) {
	//	//get_all_suite();
	//	test_80();
	//}
	//return 0;

	//test_polymorphic_vector();
	//return 0;

	//test_old();

	LoopData data;
	data.m_shader_suite = get_all_suite();

	auto before_clear = [&] {
		data.m_previous_rendering.resize(data.m_w_screen, data.m_h_screen);

		auto tmp = GLptr(
			[](GLuint* ptr) { glGenFramebuffers(1, ptr); },
			[](const GLuint* ptr) { glDeleteFramebuffers(1, ptr); }
		);
		glBindFramebuffer(GL_FRAMEBUFFER, tmp);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, data.m_previous_rendering.m_gl, 0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, data.m_w_screen, data.m_h_screen, 0, data.m_h_screen, data.m_w_screen, 0, GL_COLOR_BUFFER_BIT, GL_LINEAR);

		data.m_previous_rendering.bind();
		glGenerateMipmap(GL_TEXTURE_2D);
	};

	create_context_and_run(before_clear, [&](GLFWwindow* window) {
		glfwGetFramebufferSize(window, &data.m_w_screen, &data.m_h_screen);
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
