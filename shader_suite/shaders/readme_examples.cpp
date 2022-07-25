#include "readme_examples.h"

#include <type_traits>

csl::glsl::frag_420::Shader types_operators_example()
{
	using namespace csl::glsl::frag_420;
	Shader shader;

	Uint counter = 0;
	vec2 uv = vec2(1.0, 2.0);
	vec4 color = vec4(0.0, uv, Float(counter));
	bvec3 m = not(bvec3(!Bool(true), false, false));

	return shader;
}

csl::glsl::frag_420::Shader manual_naming_example()
{
	using namespace csl::glsl::frag_420;
	Shader shader;

	// Naming during variable declaration.
	Qualify<Uniform, vec3> eye("eye");
	Qualify<In, vec3> normal("normal");
	Qualify<In, vec3> position("position");
	Qualify<Out, Float> result("result");

	shader.main([&] {
		// Naming during variable initialisation.
		Float alpha = Float(1.2) << "alpha";
		vec3 V = eye - position << "V";
		vec3 N = normalize(normal) << "N";

		result = alpha * dot(N, V);
		});

	return shader;
}

csl::glsl::frag_420::Shader auto_naming_example()
{
	using namespace csl::glsl::frag_420;
	Shader shader;

	Qualify<Uniform, vec3> eye;
	Qualify<In, vec3> normal;
	Qualify<In, vec3> position;
	Qualify<Out, Float> result;

	shader.main([&] {
		Float alpha = 1.2;
		vec3 V = normalize(eye - position);
		vec3 N = normalize(normal);

		result = alpha * dot(N, V);
		});

	return shader;
}

csl::glsl::frag_420::Shader swizzling_example()
{
	using namespace csl::glsl::frag_420;
	using namespace csl::swizzles::all;
	Shader shader;

	vec4 col("col");
	vec4 out("out");

	col = CSL_TERNARY(col(a) > 0, col, 1.0 - col);

	// Can you guess what is actually assigned?
	out(a) = col(b, a, r)(y, z)(s);

	return shader;
}

csl::glsl::frag_420::Shader qualifier_example()
{
	using namespace csl::glsl::frag_420;
	Shader shader;

	Qualify<Out, vec4> color("color");
	Qualify<Layout<Location<4>>, In, vec3> position("position");
	Qualify<Layout<Binding<0>>, Uniform, sampler2DArray, Array<8>> samplers("samplers");

	return shader;
}

csl::glsl::frag_420::Shader arrays_example()
{
	using namespace csl::glsl::frag_420;
	Shader shader;

	// Array declaration.
	Qualify<vec3, Array<5>> vec3x5("vec3x5");

	// Array initialization.
	Qualify<Float, Array<0>> floatX = Qualify<Float, Array<0>>(0.0, 1.0, 2.0) << "floatX";

	// Multi dimensionnal arrays.
	Qualify<mat3, Array<2, 2>> mat3x2x2 = Qualify<mat3, Array<2, 2>>(
		Qualify<mat3, Array<2>>(mat3(0), mat3(1)),
		Qualify<mat3, Array<2>>(mat3(2), mat3(3))) << "mat3x2x2";

	// Usage.
	vec3x5[0] = floatX[1] * mat3x2x2[0][0] * vec3x5[1];

	return shader;
}


csl::glsl::frag_420::Shader functions_example()
{
	using namespace csl::glsl::frag_420;
	Shader shader;

	// Empty function.
	auto fun = define_function<void>([] {});

	// Named function with named parameters.
	auto add = define_function<vec3>("add",
		[](vec3 a = "a", vec3 b = "b") {
			CSL_RETURN(a + b);
		});

	// Function with some named parameters.
	auto addI = define_function<Int>(
		[](Int a, Int b = "b", Int c = "") {
			CSL_RETURN(a + b + c);
		});

	// Function calling another function.
	auto sub = define_function<vec3>([&](vec3 a, Qualify<Inout, vec3> b = "b") {
		fun();
		CSL_RETURN(add(a, -b));
		});

	// Named function with several overloads.
	auto square = define_function<vec3, ivec3, void>("square",
		[](vec3 a = "a") {
			CSL_RETURN(a * a);
		},
		[](ivec3 b = "b") {
			CSL_RETURN(b * b);
		},
			[] { CSL_RETURN; }
		);

	return shader;
}

csl::glsl::frag_420::Shader control_blocks_example()
{
	using namespace csl::glsl::frag_420;
	Shader shader;

	// Empty for.
	CSL_FOR(;;) { CSL_BREAK; }

	// For loop with named iterator.
	CSL_FOR(Int i = Int(0) << "i"; i < 5; ++i) {
		CSL_IF(i == 3) {
			++i;
			CSL_CONTINUE;
		} CSL_ELSE_IF(i < 3) {
			i += 3;
		} CSL_ELSE{
			CSL_FOR(; i > 1;)
				--i;
		}
	}

	// Not possible as i is still in the scope.
	// Int i; 

	{
		CSL_FOR(Int j = Int(0) << "j"; j < 5;) {
			CSL_WHILE(j != 3) {
				++j;
			}
		}
	}
	// OK since previous for was put in a scope.
	Int j = Int(0) << "j";

	CSL_SWITCH(j) {
		CSL_CASE(0) : { CSL_BREAK; }
		CSL_CASE(2) : { j = 3; }
	CSL_DEFAULT: { j = 2; }
	}

	return shader;
}

csl::glsl::frag_420::Shader structs_examples()
{
	using namespace csl::glsl::frag_420;
	Shader shader;

	// Struct declaration.
	CSL_STRUCT(Block,
		(mat4, mvp),
		(vec4, center)
	);

	// Nested struct.
	CSL_STRUCT(BigBlock,
		(Block, inner_block),
		(vec4, center)
	);

	// Usage.
	BigBlock big_block("big_block");
	Block block = Block(mat4(1), vec4(0)) << "block";

	block.center = big_block.inner_block.mvp * big_block.center;

	return shader;
}

csl::glsl::frag_420::Shader interface_examples()
{
	using namespace csl::glsl::frag_420;
	Shader shader;

	// Unnamed interface block.
	CSL_UNNAMED_INTERFACE_BLOCK(In, SimpleInterface,
		(Float, delta_time)
	);

	// Named array interface with multiple qualifiers.
	CSL_INTERFACE_BLOCK((Layout<Binding<0>>, Out, Array<3>), Output, out,
		(vec3, position),
		(vec3, velocity)
	);

	out[0].position += delta_time * out[0].velocity;

	return shader;
}

csl::glsl::frag_420::Shader struct_interface_comma_examples()
{
	using namespace csl::glsl::frag_420;
	Shader shader;

	using vec4x16 = Qualify<vec4, Array<16>>;
	CSL_INTERFACE_BLOCK(
		(Layout<Binding<0>, Std140>, Uniform, Array<2>), // Extra parenthesis. 
		MyInterface, vars,
		(vec4x16, vecs),								 // Typename alias.
		((Qualify<mat4, Array<4>>), myMats)				 // Extra parenthesis.
	);

	return shader;
}

csl::glsl::frag_420::Shader shader_stage_options()
{
	using namespace csl::glsl::frag_420;
	Shader shader;

	{
		// In a fragment shader:
		shader_stage_option<Layout<Early_fragment_tests>, In>();
	}

	{
		using namespace csl::glsl::geom_420;
		// In a geometry shader:
		shader_stage_option<Layout<Triangles>, In>();
		shader_stage_option<Layout<Line_strip, Max_vertices<2>>, Out>();
	}

	return shader;
}

template<typename T>
auto shader_variation(T&& parameter, std::array<double, 2> direction, bool gamma_correction)
{
	using namespace csl::glsl::frag_420;
	using namespace csl::swizzles::rgba;
	Shader shader;

	Qualify<sampler2D, Uniform> samplerA("samplerA"), samplerB("samplerB");
	Qualify<vec2, In> uvs("uvs");
	Qualify<vec4, Out> color("color");

	shader.main([&] {
		vec2 sampling_dir = vec2(direction[0], direction[1]) << "sampling_dir";

		constexpr int N = T::value;
		Qualify<vec4, Array<2 * N + 1>> cols("cols");

		CSL_FOR(Int i = Int(-N) << "i"; i <= N; ++i) {
			cols[N + i] = vec4(0);
			for (auto& sampler : { samplerA, samplerB }) {
				cols[N + i] += texture(sampler, uvs + Float(i) * sampling_dir);
			}
			color += cols[N + i] / Float(2 * N + 1);
		}

		if (gamma_correction) {
			color(r, g, b) = pow(color(r, g, b), vec3(2.2));
		}
		});

	return shader;
};

csl::glsl::frag_420::Shader meta_variation_1()
{
	return shader_variation(std::integral_constant<int, 9>{}, { 0, 1 }, true);
}

csl::glsl::frag_420::Shader meta_variation_2()
{
	return shader_variation(std::integral_constant<int, 5>{}, { 1, 0 }, false);
}

