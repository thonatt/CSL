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
