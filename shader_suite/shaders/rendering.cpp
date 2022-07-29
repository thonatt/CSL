#include "rendering.h"


csl::glsl::frag_420::Shader textured_mesh_frag()
{
	using namespace csl::glsl::frag_420;
	Shader shader;

	CSL_INTERFACE_BLOCK(in, VertexData, frag_in,
		(vec3, position),
		(vec3, normal),
		(vec3, color),
		(vec2, uv)
	);

	Qualify<layout<binding<0>>, uniform, sampler2D> tex("tex");

	Qualify<layout<location<0>>, out, vec4> color("color");

	shader.main([&] {
		color = texture(tex, frag_in.uv);
	});
	return shader;
}

csl::glsl::frag_420::Shader multiple_outputs_frag()
{
	using namespace csl::glsl::frag_420;
	using namespace csl::swizzles::all;
	Shader shader;

	CSL_INTERFACE_BLOCK(in, VertexData, frag_in,
		(vec3, position),
		(vec3, normal),
		(vec3, color),
		(vec2, uv)
	);

	Qualify<sampler2D, layout<binding<0>>, uniform> tex("tex");

	Qualify<layout<location<0>>, out, vec3> out_position("out_position");
	Qualify<layout<location<1>>, out, vec3> out_normal("out_normal");
	Qualify<layout<location<2>>, out, vec2> out_uv("out_uv");
	Qualify<layout<location<3>>, out, Float> out_depth("out_depth");
	Qualify<layout<location<4>>, out, vec3> out_tex("out_tex");

	shader.main([&] {
		out_position = 0.5 * (1.0 + frag_in.position);
		out_normal = 0.5 * (1.0 + normalize(frag_in.normal));
		out_uv = frag_in.uv;
		out_tex = texture(tex, frag_in.uv)(x, y, z);
		out_depth = gl_FragCoord(z);
	});

	return shader;
}

csl::glsl::geom_420::Shader geometric_normals() {
	using namespace csl::glsl::geom_420;

	Shader shader;

	shader_stage_option<layout<triangles>, in>();
	shader_stage_option<layout<line_strip, max_vertices<2>>, out>();

	CSL_INTERFACE_BLOCK((in, Array<0>), VertexData, vertex_in,
		(vec3, position),
		(vec3, normal),
		(vec3, color),
		(vec2, uv)
	);

	Qualify<uniform, mat4> view("view");
	Qualify<uniform, mat4> proj("proj");

	shader.main([&] {
		vec3 a = vertex_in[0].position;
		vec3 b = vertex_in[1].position;
		vec3 c = vertex_in[2].position;

		vec3 tri_normal = normalize(cross(b - a, c - b));
		vec3 tri_center = (a + b + c) / 3.0;

		gl_Position = proj * view * vec4(tri_center, 1.0);
		EmitVertex();
		gl_Position = proj * view * vec4(tri_center + 0.05 * tri_normal, 1.0);
		EmitVertex();
		EndPrimitive();
	});

	return shader;
}

csl::glsl::frag_420::Shader single_color_frag() {
	using namespace csl::glsl::frag_420;

	Shader shader;
	Qualify<layout<location<0>>, out, vec4> color("color");

	shader.main([&] {
		color = vec4(1.0, 0.0, 1.0, 1.0);
	});

	return shader;
}

csl::glsl::tcs_420::Shader tessellation_control_shader_example() {
	using namespace csl::glsl::tcs_420;

	Shader shader;
	shader_stage_option<layout<vertices<3>>, out>();

	CSL_INTERFACE_BLOCK((in, Array<0>), VertexData, tcs_in,
		(vec3, position),
		(vec3, normal),
		(vec3, color),
		(vec2, uv)
	);

	CSL_INTERFACE_BLOCK((out, Array<0>), VertexData, tcs_out,
		(vec3, position),
		(vec3, normal),
		(vec3, color),
		(vec2, uv)
	);

	Qualify<uniform, Float> tessellation_amount("tessellation_amount");

	shader.main([&] {

		CSL_IF(gl_InvocationID == 0) {
			gl_TessLevelInner[0] = tessellation_amount;
			gl_TessLevelOuter[0] = tessellation_amount;
			gl_TessLevelOuter[1] = tessellation_amount;
			gl_TessLevelOuter[2] = tessellation_amount;
		}

		tcs_out[gl_InvocationID].position = tcs_in[gl_InvocationID].position;
		tcs_out[gl_InvocationID].normal = tcs_in[gl_InvocationID].normal;
		tcs_out[gl_InvocationID].color = tcs_in[gl_InvocationID].color;
		tcs_out[gl_InvocationID].uv = tcs_in[gl_InvocationID].uv;
		gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	});

	return shader;
}

csl::glsl::tev_420::Shader tessellation_evaluation_shader_example()
{
	using namespace csl::glsl::tev_420;
	using namespace csl::swizzles::xyzw;

	Shader shader;

	CSL_INTERFACE_BLOCK((in, Array<0>), VertexData, tev_in,
		(vec3, position),
		(vec3, normal),
		(vec3, color),
		(vec2, uv)
	);

	CSL_INTERFACE_BLOCK(out, VertexData, tev_out,
		(vec3, position),
		(vec3, normal),
		(vec3, color),
		(vec2, uv)
	);

	shader_stage_option<layout<triangles, equal_spacing, ccw>, in>();
	Qualify<layout<binding<0>>, uniform, sampler2D> displacement_tex("displacement_tex");
	Qualify<uniform, mat4> view("view");
	Qualify<uniform, mat4> proj("proj");

	shader.main([&] {
		tev_out.normal = normalize(gl_TessCoord[0] * tev_in[0].normal + gl_TessCoord[1] * tev_in[1].normal + gl_TessCoord[2] * tev_in[2].normal);
		tev_out.color = gl_TessCoord[0] * tev_in[0].color + gl_TessCoord[1] * tev_in[1].color + gl_TessCoord[2] * tev_in[2].color;
		tev_out.uv = gl_TessCoord[0] * tev_in[0].uv + gl_TessCoord[1] * tev_in[1].uv + gl_TessCoord[2] * tev_in[2].uv;

		vec3 pos = gl_TessCoord[0] * tev_in[0].position + gl_TessCoord[1] * tev_in[1].position + gl_TessCoord[2] * tev_in[2].position;
		Float displacement = texture(displacement_tex, tev_out.uv)(x) << "displacement";
		vec4 delta_pos = 2.0 * vec4(displacement * tev_out.normal, 0.0);
		tev_out.position = pos + displacement * delta_pos(x, y, z);
		gl_Position = proj * view * vec4(tev_out.position, 1.0);
	});

	return shader;
}

csl::glsl::frag_420::Shader phong_shading_frag() {
	using namespace csl::glsl::frag_420;
	Shader shader;

	CSL_INTERFACE_BLOCK(in, VertexData, frag_in,
		(vec3, position),
		(vec3, normal),
		(vec3, color),
		(vec2, uv)
	);

	Qualify<uniform, vec3> eye("eye");

	Qualify<out, vec4> out_color("out_color");

	CSL_STRUCT(Light,
		(vec3, position),
		(vec3, color)
	);

	using LightArray4 = Qualify<Light, Array<4>>;
	LightArray4 lights = LightArray4(
		Light(vec3(4.0, 1.0, -2.0), vec3(1.0, 0.0, 0.0)),
		Light(vec3(1.0, 4.0, -2.0), vec3(0.0, 1.0, 0.0)),
		Light(vec3(4.0, 4.0, -2.0), vec3(0.0, 0.0, 1.0)),
		Light(vec3(-10.0, -10.0, -2.0), vec3(1.0, 1.0, 1.0))
	) << "lights";

	shader.main([&] {
		const Float kd = 0.4;
		const Float ks = 0.3;
		const vec3 base_color = vec3(0.5);

		const vec3 N = normalize(frag_in.normal);
		const vec3 V = normalize(eye - frag_in.position);

		vec3 col = (1.0 - kd - ks) * base_color;
		CSL_FOR(Int i = 0; i < 4; ++i) {
			const vec3 L = normalize(lights[i].position - frag_in.position);
			const vec3 R = reflect(L, N);
			const Float diffuse = max(0.0, dot(L, N));
			const Float specular = max(0.0, dot(R, V));
			col += (kd * diffuse + ks * specular) * lights[i].color;
		}

		out_color = vec4(col, 1.0);
	});

	return shader;
}

csl::glsl::vert_420::Shader interface_vertex_shader()
{
	using namespace csl::glsl::vert_420;
	using namespace csl::swizzles::all;
	Shader shader;

	Qualify<layout<location<0>>, in, vec3> in_position("in_position");
	Qualify<layout<location<1>>, in, vec2> in_uv("in_uv");
	Qualify<layout<location<2>>, in, vec3> in_normal("in_normal");

	CSL_INTERFACE_BLOCK(out, VertexData, vertex_out,
		(vec3, position),
		(vec3, normal),
		(vec3, color),
		(vec2, uv)
	);

	Qualify<uniform, mat4> view("view");
	Qualify<uniform, mat4> proj("proj");

	shader.main([&] {
		vertex_out.position = in_position;
		vertex_out.normal = in_normal;
		vertex_out.color = vec3(1.0, 0.0, 1.0);
		vertex_out.uv = in_uv;
		gl_Position = proj * (view * vec4(in_position, 1.0));

	});
	return shader;
}

csl::glsl::vert_420::Shader screen_quad_vertex_shader()
{
	using namespace csl::glsl::vert_420;
	using namespace csl::swizzles::xyzw;
	Shader shader;

	Qualify<layout<location<0>>, in, vec3> pos("pos");
	Qualify<layout<location<1>>, in, vec2> in_uv("in_uv");
	Qualify<out, vec2> uv("uv");

	shader.main([&]
	{
		gl_Position = vec4(2.0 * pos - 1.0, 1.0);
		uv = in_uv;
	});

	return shader;
}