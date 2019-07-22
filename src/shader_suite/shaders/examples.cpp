#include "examples.h"

#include <csl/Shaders.h>

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

std::string phongShading_automatic_naming()
{
	using namespace csl::frag_330;

	Shader shader;

	Out<vec4, Layout<Location<0>>> out_color;

	Uniform<vec3> light_position, eye_position;
	Uniform<vec3> specular_col, diffuse_col, ambient_col;
	Uniform<Float> k_s, k_d, k_a, shininess;

	In<vec3> color, normal, position;

	shader.main([&] {
		vec3 L = normalize(light_position - position);
		vec3 V = normalize(eye_position - position);
		vec3 N = normalize(normal);
		vec3 R = -reflect(L, N);
		vec3 diffuse = max(0.0, dot(L, N))*diffuse_col;
		vec3 specular = pow(max(0.0, dot(R, V)), shininess)*specular_col ;
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
