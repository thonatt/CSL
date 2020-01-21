#include "examples.h"

#include <csl/Core.hpp>

void auto_naming_example()
{
	using namespace csl::frag_450;

	{
		Shader shader;

		//naming during variable declaration
		In<vec3> normal("normal");
		In<vec3> position("position");
		Uniform<vec3> eye("eye");

		shader.main([&] {
			//naming during variable initialisation
			Float alpha = Float(1.2) << "alpha";
			vec3 V = eye - position << "V";
			vec3 N = normalize(normal) << "N";

			//naming during variable declaration
			Float result("result");
			result = alpha * dot(N, V);
		});

		std::cout << shader.str() << std::endl;
	}

	{
		Shader shader;

		In<vec3> normal;
		In<vec3> position;
		Uniform<vec3> eye;

		shader.main([&] {
			Float alpha = 1.2;
			vec3 V = normalize(eye - position);
			vec3 N = normalize(normal);

			Float result;
			result = alpha * dot(N, V);
		});

		std::cout << shader.str() << std::endl;
	}

}

void swizzling_example()
{
	using namespace csl::frag_330;
	using namespace csl::swizzles::rgba;

	Shader shader;

	mat4 cols("cols");
	vec4 col("col");
	vec4 out("out");

	cols[0] = CSL_TERNARY(col[r] > 0, col, 1.0 - col);

	//can you guess what is actually assigned ?
	out[a] = col[a, b, g][b, g][r];

	std::cout << shader.str() << std::endl;
}

void qualifier_example()
{
	using namespace csl::frag_330;

	Shader shader;

	Out<vec4> color("color");
	In<vec3, Layout<Location<4>>> position("position");
	Uniform<Array<sampler2DArray, 8>, Layout<Binding<0>>> samplers("samplers");

	//in case of multiple occurences, last one prevails
	Uniform<mat4, Layout<Location<0>, Row_major, Location<1>> > mvp("mvp");

	std::cout << shader.str() << std::endl;
}

void arrays_example()
{
	using namespace csl::frag_330;

	Shader shader;

	Array<vec3, 5> vec3A("myVec3A");
	Array<Float> floatA = Array<Float>(0.0, 1.0, 2.0) << "floatA";

	Array<Array<mat3, 2>, 2> matA = Array<Array<mat3, 2>, 2>(
		Array<mat3, 2>(mat3(0), mat3(1)),
		Array<mat3, 2>(mat3(2), mat3(3))
		) << "matA";

	vec3A[0] = floatA[1] * matA[0][0]* vec3A[1];

	std::cout << shader.str() << std::endl;
}

void functions_example()
{
	using namespace csl::frag_330;

	Shader shader;

	//empty function
	auto fun = declareFunc<void>([] {});

	//named function with named parameters
	auto add = declareFunc<vec3>("add",
		[](vec3 a = "a", vec3 b = "b") {
			CSL_RETURN(a + b);
		}
	);

	//function with some named parameters
	auto addI = declareFunc<Int>(
		[](Int a, Int b = "b", Int c = "") {
			CSL_RETURN(a + b + c);
		}
	);

	////function calling another function
	auto sub = declareFunc<vec3>([&](vec3 a, vec3 b) {
		fun();
		CSL_RETURN(add(a, -b));
	});

	//named function with overload
	auto square = declareFunc<vec3, ivec3, void>( "square",
		[](vec3 a = "a") {
			CSL_RETURN(a*a);
		}, 
		[](ivec3 b = "b") {
			CSL_RETURN(b*b);
		},
		[] { CSL_RETURN; }
	);

	std::cout << shader.str() << std::endl;
}

void structure_stratements_example()
{
	using namespace csl::frag_330;

	Shader shader;

	//empty for
	CSL_FOR(;;) { CSL_BREAK; }

	//named function with named parameters
	CSL_FOR(Int i = Int(0) << "i"; i < 5; ++i) {
		CSL_IF(i == 3) {
			++i;
			CSL_CONTINUE;
		} CSL_ELSE_IF(i < 3) {
			i += 3;
		} CSL_ELSE {
			CSL_FOR(; i > 1;)
				--i;
		}
	}
	//Not possible as i is still in the scope
	//Int i; 

	{
		CSL_FOR(Int j = Int(0) << "j"; j < 5;) {
			CSL_WHILE(j != 3) {
				++j;
			}
		}
	}
	//OK since previous for was put in a scope
	Int j("j");

	CSL_SWITCH(j) {
		CSL_CASE(0) : { CSL_BREAK; }
		CSL_CASE(2) : { j = 3; }
		CSL_DEFAULT: { j = 2; }
	}

	std::cout << shader.str() << std::endl;
}

void structs_examples() {
	using namespace csl::frag_330;

	Shader shader;

	//struct declaration
	CSL_STRUCT(MyBlock,
		(mat4) mvp,
		(vec4) center
	);

	//nested struct
	CSL_STRUCT(MyBigBlock,
		(MyBlock) innerBlock,
		(vec4) center
	);

	//usage
	MyBigBlock bigBlock("block");
	MyBlock block = MyBlock(mat4(1), vec3(0)) << "block";

	block.center = bigBlock.innerBlock.mvp*block.center;

	std::cout << shader.str() << std::endl;
}

void interface_examples()
{
	using namespace csl::frag_330;

	Shader shader;

	//unnamed interface
	CSL_INTERFACE_BLOCK(In<>, SimpleInterface, , ,
		(Float) current_time
	);

	//named array interface with qualifiers
	CSL_INTERFACE_BLOCK(Out<Layout<Binding<0>>>, Output, out, 3,
		(vec3) position,
		(Float) velocity
	);

	out[0].position += current_time * out[0].velocity;

	std::cout << shader.str() << std::endl;
}

void struct_interface_comma_examples()
{
	using namespace csl::frag_330;

	Shader shader;

	using MyQualifier = Uniform<Layout<Binding<0>, Std140>>;
	using vec4x16 = Array<vec4, 16>;
	CSL_INTERFACE_BLOCK(MyQualifier, MyInterface, vars, 2,
		(vec4x16) myVecs,
		(Array<mat4>::Size<4>) myMats
	);

	std::cout << shader.str() << std::endl;
}

void shader_stage_options()
{	
	{
		using namespace csl::frag_330;

		Shader shader;
		//in a fragment shader
		in<Layout<Early_fragment_tests>>();

		std::cout << shader.str() << std::endl;
	}

	{
		using namespace csl::geom_330;
		Shader shader;

		//in a geometry shader
		in<Layout<Triangles>>();
		out<Layout<Line_strip, Max_vertices<2>>>();

		std::cout << shader.str() << std::endl;
	}
}

void meta_variations()
{
	auto shader_variation =
		[](auto template_parameter, double sampling_angle, bool gamma_correction) 
	{
			
		using namespace csl::frag_430;
		using namespace csl::swizzles::rgba;

		Shader shader;
		Uniform<sampler2D> samplerA("samplerA"), samplerB("samplerB");
		In<vec2> uvs("uvs");
		Out<vec4> color("color");

		shader.main([&] {
			vec2 sampling_dir = vec2(cos(sampling_angle), sin(sampling_angle)) << "sampling_dir";

			constexpr int N = decltype(template_parameter)::value;
			Array<vec4, 2 * N + 1> cols("cols");
			CSL_FOR(Int i = Int(-N) << "i"; i <= N; ++i) {
				cols[N + i] = vec4(0);
				for (auto& sampler : { samplerA, samplerB }) {
					cols[N + i] += texture(sampler, uvs + i * sampling_dir);
				}
				color += cols[N + i] / Float(2 * N + 1);
			}

			if (gamma_correction) {
				color[r, g, b] = pow(color[r, g, b], vec3(2.2));
			}
		});

		std::cout << shader.str() << std::endl;
	};
 
	shader_variation(csl::ConstExpr<int, 9>{}, 0, true);
	shader_variation(csl::ConstExpr<int,5>{}, 1.57079632679, false);
}

std::string phongShading() {
	using namespace csl::frag_330;

	Shader shader;

	Out<vec4, Layout<Location<0>>> out_color("out_color");

	Uniform<vec3> light_position("light_position"), eye_position("eye_position");
	Uniform<vec3> specular_col("specular_col"), diffuse_col("diffuse_col"), ambient_col("ambient_col");
	Uniform<Float> k_s("k_s"), k_d("k_d"), k_a("k_a"), shininess("shininess");

	In<vec3> color("color"), normal("normal"), position("position");

	shader.main([&] {
		vec3 L = normalize(light_position - position) << "L";
		vec3 V = normalize(eye_position - position) << "V";
		vec3 N = normalize(normal) << "N";
		vec3 R = -reflect(L, N) << "R";
		vec3 diffuse = max(0.0, dot(L, N))*diffuse_col << "diffuse";
		vec3 specular = pow(max(0.0, dot(R, V)), shininess)*specular_col << "specular";
		out_color = vec4(k_a*ambient_col + k_d * diffuse + k_s * specular, 1.0);
	});

	return shader.str();
}

std::string per_triangle_normal_geom()
{
	using namespace csl::geom_330;
	using namespace csl::swizzles::xyzw;

	Shader shader;

	in<Layout<Triangles>>();
	out<Layout<Line_strip, Max_vertices<2>>>();

	Uniform<mat4> MVP("MVP");
	Uniform<Float> size("size");

	shader.main([&] {
		vec3 v_1 = gl_in[0].gl_Position[x, y, z] << "a";
		vec3 v_2 = gl_in[1].gl_Position[x, y, z] << "b";
		vec3 v_3 = gl_in[2].gl_Position[x, y, z] << "c";

		vec3 center = (v_1 + v_2 + v_3) / 3.0 << "center";
		vec3 normal = normalize(cross(v_2 - v_1, v_3 - v_2)) << "normal";
		gl_Position = MVP * vec4(center, 1.0);
		EmitVertex();
		gl_Position = MVP * vec4(center + size * normal, 1.0);
		EmitVertex();
		EndPrimitive();
	});

	return shader.str();
}
