#include <iostream>

#include "AlgebraFunctions.h"
#include "Layouts.h"
#include "Samplers.h"
#include "BuildingBlocks.h"
#include "Shaders.h"


int main()
{
	//testFuns();
	//testBlocks();

	using namespace fs;
	{
		FragmentShader shader;

		GL_STRUCT(Tri,
			(Float) angle,
			(vec3) center,
			(mat4x4) proju
		);

		GL_STRUCT(MegaTri,
			(Tri) triA,
			(Tri) triB,
			(Tri) triC
		);

		GL_STRUCT(MegaTron,
			(Tri) triA,
			(MegaTri) mega,
			(Tri) triC
		);

		
		In<vec3> normal("normal"), position("position"), color("color");
		Uniform<vec3> lightPos("lightPos");
		Out<vec4> outColor("outColor");
		Uniform<Tri> triangle("triangle");
		//Uniform<MegaTri, Layout<Location<3>, Binding<0> >> megatron;
		MegaTron k;
		
		auto foo = makeF("proj", [](In<mat4> proj, In<vec3> point) { return proj * vec4(point, 1.0); }, "proj", "point");
		auto goo = makeF("goo", [](vec3 a, vec3 b) { return determinant(mat3(a,b,a)); } );

		shader.main([&] {
			
			//vec3 rotatedCenter = triangle.center * triangle.angle << "rotated";
			vec3 L = normalize(lightPos - position) << "L";
			vec3 diff = color * max(dot(normal, L), 0.0) + gl_FragCoord[x, y, z] << "diff";
			//diff = dot(diff, triangle.center)*diff;
			
			GL_FOR(Int a = 0; a < 5; ++a) {
				Bool myb;
				GL_IF(myb) {
					diff[x] = goo(L, diff);
				} GL_ELSE_IF(True) {
					vec4 t = vec4(L,1.0) << "t";
					diff[x] = goo(L, foo(mat4(t, t, t, t), diff)[r, b, g])[x];
				}
				
			}
			
			outColor = vec4(color[x, z], color[x, y]);

		});

		std::cout << shader.getStr() << std::endl;
	}

	

	

	return 0;
}

void testFuns() {
	auto funct1 = makeF("mul1", [](vec2 a, Float b) { vec2 c; c = b * a; return c; });
	auto funct2 = makeF("mul2", [](vec2 a, Float b) { return b * a; });
	auto funct3 = makeF("mul3", [](vec2 a, Float b) { vec2 c; c = b * a; });
	auto funct4 = makeF("mul4", [](vec2 a, Float b) { vec2 c("c"); c = b * a; }, "bkah", "bloh");
	auto funct5 = makeF("mul5", [](vec2 a, Float b) { GL_RETURN b * a; }, "bkah", "bloh");

	vec2 vv("v2");
	Float ff("ff");
	vv = funct1(vv, ff);
	vv = funct2(vv, ff);
	funct3(vv, ff);
	funct4(vv, ff);
	vv = vec2(funct5(vv, ff)[y], funct5(vv, ff)[x]);
}

void testBlocks() {
	std::cout << std::endl << "begin loop tests" << std::endl << std::endl;

	GL_FOR(Int a("a"); a < 5; ++a) {
		Int c("c");
		Continue();
		GL_FOR(Int b(0, "b"); b < a; ++b) {
			++c;
		}
		Int e("e");
	}
	//vec4(0);
	vec3 v1("v1");
	vec3 v2("v2");
	GL_IF(all(lessThanEqual(v1, v2))) {
		Int c("c");
		//c < c;

		ivec3 v2("v");

		v2[x, z] = ivec2(c, c);

		ivec4 v4("v4");

		v4 = ivec4(ivec2(c, c), ivec2(c, c));

		(v4[x, y, z]) = (v4[b, b, g]);

		++c;

		//v4 = ivec4(ivec2(), ivec2());

		v4;
		//v4 = vec4();
		//vec4 vv = vec4(0 );


	} GL_ELSE_IF(all(greaterThanEqual(v2, v1))) {
		GL_IF(any(lessThan(v2, v1))) {
		}
		vec3 c("c");
		Float f = c[z] + length(c) << "f";

	} GL_ELSE{
		vec3 c("c");
	c[x] = length(c);
	c[x] = inversesqrt(c[x]) + sqrt(c[y]) + c[x] * c[x];
	c[z, x] += vec2(exp(c[x]), log(c[y]));
	c[x, y] = cos(c[z, z] + c[y, y] * c[z] + c[x] * c[x, x]);
	mat3 m, j;
	mat3 & mm = m;
	mm = j;

	j = c[z] * transpose(m);
	//c[x] = ( ( c[x] + c[y] ) + c[x] );
	//c[x] += c[y];
	//c[x, y] += c[x, x];
	//ivec2 ii;
	}

}