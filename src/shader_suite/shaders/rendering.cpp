#include "rendering.h"

#include "include/Samplers.hpp"
#include <include/glsl/BuiltIns.hpp>
#include "include/glsl/ToGLSL.hpp"

v2::glsl::frag_420::Shader textured_mesh_frag()
{
	using namespace v2::glsl::frag_420;
	Shader shader;

	CSL2_INTERFACE_BLOCK(In, VertexData, frag_in,
		(vec3, position),
		(vec3, normal),
		(vec3, color),
		(vec2, uv)
	);

	Qualify<sampler2D, Layout<Binding<0>>, Uniform> tex("tex");

	Qualify<vec4, Layout<Location<0>>, Out> color("color");

	shader.main([&] {
		color = texture(tex, frag_in.uv);
	});
	return shader;
}

v2::glsl::frag_420::Shader multiple_outputs_frag()
{
	using namespace v2::glsl::frag_420;
	using namespace v2::swizzles::all;
	Shader shader;

	CSL2_INTERFACE_BLOCK(In, VertexData, frag_in,
		(vec3, position),
		(vec3, normal),
		(vec3, color),
		(vec2, uv)
	);

	Qualify<sampler2D, Layout<Binding<0>>, Uniform> tex("tex");

	Qualify<vec3, Layout<Location<0>>, Out> out_position("out_position");
	Qualify<vec3, Layout<Location<1>>, Out> out_normal("out_normal");
	Qualify<vec2, Layout<Location<2>>, Out> out_uv("out_uv");
	Qualify<Float, Layout<Location<3>>, Out> out_depth("out_depth");
	Qualify<vec3, Layout<Location<4>>, Out> out_tex("out_tex");

	shader.main([&] {
		out_position = 0.5 * (1.0 + frag_in.position);
		out_normal = 0.5 * (1.0 + normalize(frag_in.normal));
		out_uv = frag_in.uv;
		out_tex = texture(tex, frag_in.uv)[x, y, z];
		out_depth = gl_FragCoord[z];
	});

	return shader;
}

v2::glsl::geom_420::Shader geometric_normals() {
	using namespace v2::glsl::geom_420;

	Shader shader;

	in<Layout<Triangles>>();
	out<Layout<Line_strip, Max_vertices<2>>>();

	CSL2_INTERFACE_BLOCK((In, Array<0>), VertexData, vertex_in,
		(vec3, position),
		(vec3, normal),
		(vec3, color),
		(vec2, uv)
	);

	Qualify<mat4, Uniform> view("view");
	Qualify<mat4, Uniform> proj("proj");

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

v2::glsl::frag_420::Shader single_color_frag() {
	using namespace v2::glsl::frag_420;

	Shader shader;
	Qualify<vec4, Layout<Location<0>>, Out> color("color");

	shader.main([&] {
		color = vec4(1.0, 0.0, 1.0, 1.0);
	});

	return shader;
}

v2::glsl::tcs_420::Shader tessellation_control_shader_example() {
	using namespace v2::glsl::tcs_420;

	Shader shader;
	out<Layout<Vertices<3>>>();

	CSL2_INTERFACE_BLOCK((In, Array<0>), VertexData, tcs_in,
		(vec3, position),
		(vec3, normal),
		(vec3, color),
		(vec2, uv)
	);

	CSL2_INTERFACE_BLOCK((Out, Array<0>), VertexData, tcs_out,
		(vec3, position),
		(vec3, normal),
		(vec3, color),
		(vec2, uv)
	);

	Qualify<Float, Uniform> tessellation_amount("tessellation_amount");

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

v2::glsl::tev_420::Shader tessellation_evaluation_shader_example()
{
	using namespace v2::glsl::tev_420;
	using namespace v2::swizzles::xyzw;

	Shader shader;

	CSL2_INTERFACE_BLOCK((In, Array<0>), VertexData, tev_in,
		(vec3, position),
		(vec3, normal),
		(vec3, color),
		(vec2, uv)
	);

	CSL2_INTERFACE_BLOCK(Out, VertexData, tev_out,
		(vec3, position),
		(vec3, normal),
		(vec3, color),
		(vec2, uv)
	);

	in<Layout<Triangles, Equal_spacing, Ccw>>();
	Qualify<sampler2D, Layout<Binding<0>>, Uniform> displacement_tex("displacement_tex");
	Qualify<mat4, Uniform> view("view");
	Qualify<mat4, Uniform> proj("proj");

	shader.main([&] {
		tev_out.normal = normalize(gl_TessCoord[0] * tev_in[0].normal + gl_TessCoord[1] * tev_in[1].normal + gl_TessCoord[2] * tev_in[2].normal);
		tev_out.color = gl_TessCoord[0] * tev_in[0].color + gl_TessCoord[1] * tev_in[1].color + gl_TessCoord[2] * tev_in[2].color;
		tev_out.uv = gl_TessCoord[0] * tev_in[0].uv + gl_TessCoord[1] * tev_in[1].uv + gl_TessCoord[2] * tev_in[2].uv;

		vec3 pos = gl_TessCoord[0] * tev_in[0].position + gl_TessCoord[1] * tev_in[1].position + gl_TessCoord[2] * tev_in[2].position;
		Float displacement = texture(displacement_tex, tev_out.uv)[x] << "displacement";
		vec4 delta_pos = 2.0 * vec4(displacement * tev_out.normal, 0.0);
		tev_out.position = pos + displacement * delta_pos[x, y, z];
		gl_Position = proj * view * vec4(tev_out.position, 1.0);
	});

	return shader;
}

v2::glsl::frag_420::Shader phong_shading_frag() {
	using namespace v2::glsl::frag_420;
	Shader shader;

	CSL2_INTERFACE_BLOCK(In, VertexData, frag_in,
		(vec3, position),
		(vec3, normal),
		(vec3, color),
		(vec2, uv)
	);

	Qualify<vec3, Uniform> eye("eye");

	Qualify<vec4, Out> out_color("out_color");

	CSL2_STRUCT(Light,
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
		CSL_FOR(Int i = 0; i < LightArray4::ComponentCount; ++i) {
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

v2::glsl::vert_420::Shader interface_vertex_shader()
{
	using namespace v2::glsl::vert_420;
	using namespace v2::swizzles::all;
	Shader shader;

	Qualify<vec3, Layout<Location<0>>, In> in_position("in_position");
	Qualify<vec2, Layout<Location<1>>, In> in_uv("in_uv");
	Qualify<vec3, Layout<Location<2>>, In> in_normal("in_normal");

	CSL2_INTERFACE_BLOCK(Out, VertexData, vertex_out,
		(vec3, position),
		(vec3, normal),
		(vec3, color),
		(vec2, uv)
	);

	Qualify<mat4, Uniform> view("view");
	Qualify<mat4, Uniform> proj("proj");

	shader.main([&] {
		vertex_out.position = in_position;
		vertex_out.normal = in_normal;
		vertex_out.color = vec3(1.0, 0.0, 1.0);
		vertex_out.uv = in_uv;
		gl_Position = proj * (view * vec4(in_position, 1.0));

	});
	return shader;
}

v2::glsl::vert_420::Shader screen_quad_vertex_shader()
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