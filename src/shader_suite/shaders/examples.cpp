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

	vec4 col("col");
	vec4 out("out");
	out[b, g, r] = col[r, g, b];
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
	auto fun = declareFunc<void>([] { });

	//named function with named parameters
	auto add = declareFunc<vec3>("add",
		[](vec3 a = "a", vec3 b = "b") {
			GL_RETURN(a + b);
		}
	);

	//function with some named parameters
	auto addI = declareFunc<Int>(
		[](Int a, Int b = "b", Int c = "") {
			GL_RETURN(a + b + c);
		}
	);

	////function calling another function
	auto sub = declareFunc<vec3>([&](vec3 a, vec3 b) {
		fun();
		GL_RETURN(add(a, -b));
	});

	//named function with overload
	auto square = declareFunc<vec3, ivec3, void>( "square",
		[](vec3 a = "a") {
			GL_RETURN(a*a);
		}, 
		[](ivec3 b = "b") {
			GL_RETURN(b*b);
		},
		[] { GL_RETURN; }
	);

	std::cout << shader.str() << std::endl;
}

void structure_stratements_example()
{
	using namespace csl::frag_330;

	Shader shader;

	//empty for
	GL_FOR(;;) { GL_BREAK; }

	//named function with named parameters
	GL_FOR(Int i = Int(0) << "i"; i < 5; ++i) {
		GL_IF(i == 3) {
			++i;
			GL_CONTINUE;
		} GL_ELSE_IF(i < 3) {
			i += 3;
		} GL_ELSE {
			GL_FOR(; i > 1;)
				--i;
		}
	}
	//Not possible as i is still in the scope
	//Int i; 

	{
		GL_FOR(Int j = Int(0) << "j"; j < 5;) {
			GL_WHILE(j != 3) {
				++j;
			}
		}
	}
	//OK since previous for was put in a scope
	Int j("j");

	GL_SWITCH(j) {
		GL_CASE(0) : { GL_BREAK; }
		GL_CASE(2) : { j = 3; }
		GL_DEFAULT: { j = 2; }
	}

	std::cout << shader.str() << std::endl;
}

void structs_examples() {
	using namespace csl::frag_330;

	Shader shader;

	//struct declaration
	GL_STRUCT(MyBlock,
		(mat4) mvp,
		(vec4) center
	);

	//nested struct
	GL_STRUCT(MyBigBlock,
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
	GL_INTERFACE_BLOCK(In<>, SimpleInterface, , ,
		(Float) current_time
	);

	//named array interface with qualifiers
	GL_INTERFACE_BLOCK(Out<Layout<Binding<0>>>, Output, out, 3,
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

	using Quali = Uniform<Layout<Binding<0>, Std140>>;
	using vec4A = Array<vec4, 16>;
	GL_INTERFACE_BLOCK(Quali, MyInterface, vars, 2,
		(vec4A) myVecs,
		(GetArray<mat4>::Size<4>) myMats
	);

	std::cout << shader.str() << std::endl;
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
