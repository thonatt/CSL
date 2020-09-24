#include <array>
#include <cmath>
#include <iostream>
#include <chrono>
#include <filesystem>
#include <map>
#include <memory>

#include "pico_gl.hpp"

#include <imgui.h>

#include <shaders/dolphin.h>
#include <shaders/readme_examples.h>
#include <shaders/rendering.h>
#include <shaders/rendu.h>
#include <shaders/shadertoy.h>

using v3 = std::array<float, 3>;
using m4 = std::array<float, 16>;

v3 operator*(const float s, const v3& v) {
	return { s * v[0], s * v[1], s * v[2] };
}

v3 operator-(const v3& a, const v3& b) {
	return { a[0] - b[0], a[1] - b[1], a[2] - b[2] };
}

float dot(const v3& a, const v3& b) {
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

v3 cross(const v3& a, const v3& b) {
	return { a[1] * b[2] - a[2] * b[1] ,  a[2] * b[0] - a[0] * b[2] ,  a[0] * b[1] - a[1] * b[0] };
}

v3 normalize(const v3& v) {
	return (1.0f / std::hypot(v[0], v[1], v[2])) * v;
}

using Clock = std::chrono::high_resolution_clock;

template<typename F>
double get_timing(F&& f)
{
	auto start = Clock::now();
	f();
	return std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - start).count() / 1000.0;
}

struct ShaderExample
{
	template<typename ShaderCreation>
	ShaderExample(ShaderCreation&& f)
	{
		using namespace csl;

		auto start = Clock::now();
		auto shader = f();
		m_generation_timing = std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - start).count() / 1000.0;

		m_glsl_timing = get_timing([&]
		{
			GLSLData glsl_data;
			shader.print_glsl(glsl_data);
			m_glsl_str = glsl_data.stream.str();
		});

		std::swap(m_controller, shader.get_base());
	}

	GLProgram m_program;
	std::string m_glsl_str;
	csl::ShaderController m_controller;
	std::string m_name;
	double m_generation_timing, m_glsl_timing;
};

using ShaderPtr = std::shared_ptr<ShaderExample>;

enum class ShaderGroup {
	Test,
	Examples,
	Readme,
	Rendu,
	Shadertoy,
	Extra
};

enum class ShaderEnum {
	TypeAndOperatorsExample,
	ManualNamingExample,
	AutoNamingExample,
	SwizzlingExample,
	QualifiersExample,
	ArraysExample,
	FunctionsExample,
	ControlBlocksExample,
	StructsExample,
	InterfaceBlocksExamples,
	CommaAndTypenamesExample,
	ShaderStageOptionsExample,
	MetaVariation1Example,
	MetaVariation2Example,
	ScreenQuadVertex,
	InterfaceVertex,
	TexturedMeshFrag,
	FractalNoiseFrag,
	MultipleOutputsFrag,
	PhongFrag,
	GeometricNormalsGeom,
	SingleColorFrag,
	TessControl,
	TessEval,
	AtmosphereScatteringLUT,
	AtmosphereRendering,
	CSLVaporwave,
	DolphinUbershaderVert,
	DolphinUbershaderFrag
};

const std::string& shader_name(const ShaderEnum shader)
{
	static std::unordered_map<ShaderEnum, std::string> shaders_strs = {
		{ ShaderEnum::TypeAndOperatorsExample, "Types & Operators" },
		{ ShaderEnum::ManualNamingExample, "Manual naming" },
		{ ShaderEnum::AutoNamingExample, "Automatic naming" },
		{ ShaderEnum::SwizzlingExample, "Swizzling" },
		{ ShaderEnum::QualifiersExample, "Qualifiers" },
		{ ShaderEnum::ArraysExample, "Arrays" },
		{ ShaderEnum::FunctionsExample, "Functions" },
		{ ShaderEnum::ControlBlocksExample, "Control blocks" },
		{ ShaderEnum::StructsExample, "Structs" },
		{ ShaderEnum::InterfaceBlocksExamples, "Interface blocks" },
		{ ShaderEnum::CommaAndTypenamesExample, "Comma & typenames" },
		{ ShaderEnum::ShaderStageOptionsExample, "Shader stage options" },
		{ ShaderEnum::MetaVariation1Example, "Variation 1" },
		{ ShaderEnum::MetaVariation2Example, "Variation 2" },
		{ ShaderEnum::ScreenQuadVertex, "Screen quad" },
		{ ShaderEnum::InterfaceVertex, "Interface vert" },
		{ ShaderEnum::TexturedMeshFrag, "Textured mesh frag" },
		{ ShaderEnum::FractalNoiseFrag, "Fractal noise frag" },
		{ ShaderEnum::MultipleOutputsFrag, "Multiple outputs frag" },
		{ ShaderEnum::PhongFrag, "Phong shading frag" },
		{ ShaderEnum::GeometricNormalsGeom, "Geometric normals geom" },
		{ ShaderEnum::SingleColorFrag, "Single color frag" },
		{ ShaderEnum::TessControl, "Tessellation control" },
		{ ShaderEnum::TessEval, "Tessellation evaluation" },
		{ ShaderEnum::AtmosphereScatteringLUT, "Atmosphere LUT compute" },
		{ ShaderEnum::AtmosphereRendering, "Atmosphere scattering" },
		{ ShaderEnum::CSLVaporwave, "Vaporwave CSL" },
		{ ShaderEnum::DolphinUbershaderVert, "Dolphin Ubershader vert" },
		{ ShaderEnum::DolphinUbershaderFrag, "Dolphin Ubershader frag" },
	};

	return shaders_strs.find(shader)->second;
}
struct ShaderSuite {

	template<typename F>
	void add_shader(const ShaderGroup group, const ShaderEnum shader, F&& f) {
		auto shader_example = std::make_shared<ShaderExample>(std::forward<F>(f));
		shader_example->m_name = shader_name(shader);
		m_shaders.emplace(shader, shader_example);
		m_groups[group].emplace_back(shader_example);
	}

	std::map<ShaderEnum, ShaderPtr> m_shaders;
	std::unordered_map<ShaderGroup, std::vector<ShaderPtr>> m_groups;
};

struct LoopData;

struct PipelineBase {

	struct ShaderActive {
		ShaderPtr m_shader;
		bool m_active = true;
	};

	void add_shader(const GLenum type, const std::pair<ShaderEnum, ShaderPtr>& shader)
	{
		m_shaders[GLProgram::get_shader_type(type)] = { shader.second };
		m_program.set_shader_glsl(type, shader.second->m_glsl_str);
	}

	virtual void additionnal_gui() {};
	virtual void draw(LoopData& data) {};
	virtual ~PipelineBase() = default;

	GLProgram m_program;
	std::unordered_map<GLShaderType, ShaderActive> m_shaders;
};

std::unordered_map<std::string, std::shared_ptr<PipelineBase>> get_all_pipelines(const LoopData& data);

struct GlobalMetrics {
	double m_time_total = 0.0f;
	std::size_t m_characters_count = 0;
	std::size_t m_expressions_count = 0;
	std::size_t m_instructions_count = 0;
	std::size_t m_memory = 0;
};

struct LoopData {

	void init_gl_once()
	{
		using v2 = std::array<float, 2>;

		static bool first = true;
		if (!first) {
			return;
		}
		first = false;

		m_framebuffer.create_attachment(GLformat());
		m_fractal_noise.create_attachment(GLformat());
		m_scattering_LUT.create_attachment(GLformat(GL_RGBA32F, GL_RGBA, GL_FLOAT));

		std::filesystem::path path = std::string(CSL_SHADER_SUITE_RESOURCE_PATH) + "/shadertoy-font-25.png";
		m_tex_letters.load(std::filesystem::absolute(path).string());

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
			std::vector<v2>{ v2{ 0,0 }, v2{ 0,1 }, v2{ 1,1 }, v2{ 1,0 } }
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
			ns[i] = normalize(ps[i]);
			uvs[i][0] = std::acos(ns[i][2]) / pi;
			uvs[i][1] = 0.5f * (1.0f + std::atan2(ns[i][1], ns[i][0]) / pi);
		}
		m_isosphere.set_attributes(ps, uvs, ns);

		m_pipelines = get_all_pipelines(*this);
		m_current_pipeline = m_pipelines.find("csl_vaporwave")->second;
	}

	void set_mvp(const float distance, const float radius, const float near, const float far)
	{
		constexpr float two_pi = 2.0f * 3.141519265f;
		const v3 at = { 0, 0, 0 };
		const v3 up = { 0, 0, 1 };
		const float theta = std::fmod(m_time / 12.0f, 1.0f) * two_pi;
		m_eye = v3{ radius * std::cos(theta), radius * std::sin(theta), distance };

		const v3 z_axis = normalize(at - m_eye);
		const v3 x_axis = normalize(cross(up, z_axis));
		const v3 y_axis = cross(z_axis, x_axis);

		m_view_transposed = {
			x_axis[0], x_axis[1], x_axis[2], dot(x_axis, m_eye),
			y_axis[0], y_axis[1], y_axis[2], dot(y_axis, m_eye),
			z_axis[0], z_axis[1], z_axis[2], dot(z_axis, m_eye),
			0.0f, 0.0f, 0.0f, 1.0f
		};

		const float fov_deg = 90.0f;
		const float scale = 1.0f / std::tan((two_pi * fov_deg) / (4.0f * 180.0f));

		m_proj = {
			scale, 0.0f, 0.0f, 0.0f,
			0.0f, scale, 0.0f, 0.0f,
			0.0f, 0.0f,  -(far + near) / (far - near), -1.0f,
			0.0f, 0.0f,  -2.0f * far * near / (far - near), -0.0f
		};
	}

	ShaderSuite m_shader_suite;
	std::unordered_map<std::string, std::shared_ptr<PipelineBase>> m_pipelines;
	std::shared_ptr<PipelineBase> m_current_pipeline = {};
	GLTexture m_tex_letters, m_previous_rendering;
	GLFramebuffer m_framebuffer, m_fractal_noise, m_scattering_LUT;
	GLmesh m_isosphere, m_quad, m_screen_quad, m_triangle;
	GlobalMetrics m_global_metrics;
	m4 m_view_transposed, m_proj;
	v3 m_eye;
	float m_time = 0.0f;
	int m_w_screen = 0, m_h_screen = 0;
};

struct PipelineGrid final : PipelineBase {
	void draw(LoopData& data) override {
		m_program.set_uniform("time", glUniform1f, data.m_time);
		glDisable(GL_DEPTH_TEST);
		data.m_screen_quad.draw();
	}
};

struct Pipeline80 final : PipelineBase {
	void draw(LoopData& data) override {
		m_program.set_uniform("iTime", glUniform1f, data.m_time);
		m_program.set_uniform("iResolution", glUniform2f, (float)data.m_framebuffer.m_w, (float)data.m_framebuffer.m_h);
		data.m_tex_letters.bind_slot(GL_TEXTURE0);
		glDisable(GL_DEPTH_TEST);
		data.m_screen_quad.draw();
	}
};

struct PipelineTexturedMesh final : PipelineBase {
	void draw(LoopData& data) override {
		data.m_framebuffer.clear(0.3f, 0.3f, 0.3f);
		data.set_mvp(1.2f, 0.1f, 0.1f, 5.0f);
		m_program.set_uniform("view", glUniformMatrix4fv, 1, GL_TRUE, data.m_view_transposed.data());
		m_program.set_uniform("proj", glUniformMatrix4fv, 1, GL_FALSE, data.m_proj.data());
		data.m_previous_rendering.bind_slot(GL_TEXTURE0);
		data.m_quad.draw();
	}
};

struct PipelineNoise final : PipelineBase {
	void draw(LoopData& data) override {
		data.m_fractal_noise.resize(data.m_framebuffer.m_w, data.m_framebuffer.m_h);
		data.m_fractal_noise.bind_draw();
		m_program.set_uniform("freq_count", glUniform1i, m_frequencies_count);
		m_program.set_uniform("uv_scaling", glUniform1f, m_uv_scaling);
		glDisable(GL_DEPTH_TEST);
		data.m_screen_quad.draw();
		data.m_framebuffer.blit_from(data.m_fractal_noise);
	}

	void additionnal_gui() override
	{
		ImGui::SliderInt("frequencies count", &m_frequencies_count, 1, 8);
		ImGui::SliderFloat("uv scaling", &m_uv_scaling, 4.0, 64.0);
	}

	int m_frequencies_count = 4;
	float m_uv_scaling = 32.0f;
};

struct PipelinePhong final : PipelineBase {
	void draw(LoopData& data) override {
		data.set_mvp(2.5f, 2.0f, 0.8f, 5.0f);
		m_program.set_uniform("view", glUniformMatrix4fv, 1, GL_TRUE, data.m_view_transposed.data());
		m_program.set_uniform("proj", glUniformMatrix4fv, 1, GL_FALSE, data.m_proj.data());
		m_program.set_uniform("eye", glUniform3f, data.m_eye[0], data.m_eye[1], data.m_eye[2]);
		data.m_isosphere.draw();
	}
};

struct PipelineGeometry final : PipelineBase {
	void draw(LoopData& data) override {
		// render noise texture
		{
			data.m_pipelines.find("noise")->second->m_program.use();
			data.m_fractal_noise.resize(data.m_framebuffer.m_w, data.m_framebuffer.m_h);
			data.m_fractal_noise.clear();
			data.m_fractal_noise.bind_draw();
			glDisable(GL_DEPTH_TEST);
			data.m_screen_quad.draw();
			data.m_fractal_noise.m_attachments[0].generate_mipmap();
		}

		data.m_framebuffer.bind_draw();
		data.set_mvp(3.0f, 2.0f, 0.1f, 5.0f);
		glEnable(GL_DEPTH_TEST);

		// render tessellated mesh
		{
			auto& displacement_pipeline = data.m_pipelines.find("displacement")->second->m_program;
			displacement_pipeline.use();
			displacement_pipeline.set_uniform("view", glUniformMatrix4fv, 1, GL_TRUE, data.m_view_transposed.data());
			displacement_pipeline.set_uniform("proj", glUniformMatrix4fv, 1, GL_FALSE, data.m_proj.data());
			displacement_pipeline.set_uniform("tessellation_amount", glUniform1f, 100.0f);
			data.m_fractal_noise.m_attachments[0].bind_slot(GL_TEXTURE0);
			data.m_isosphere.draw(GL_PATCHES);
		}

		data.m_framebuffer.bind_draw();
		m_program.use();
		m_program.set_uniform("view", glUniformMatrix4fv, 1, GL_TRUE, data.m_view_transposed.data());
		m_program.set_uniform("proj", glUniformMatrix4fv, 1, GL_FALSE, data.m_proj.data());
		m_program.set_uniform("tessellation_amount", glUniform1f, 100.0f);
		data.m_fractal_noise.m_attachments[0].bind_slot(GL_TEXTURE0);
		data.m_isosphere.draw(GL_PATCHES);
	}
};

struct PipelineDisplacement final : PipelineBase {
	void draw(LoopData& data) override
	{
		// render noise texture
		{
			data.m_pipelines.find("noise")->second->m_program.use();
			data.m_fractal_noise.resize(data.m_framebuffer.m_w, data.m_framebuffer.m_h);
			data.m_fractal_noise.clear();
			data.m_fractal_noise.bind_draw();
			glDisable(GL_DEPTH_TEST);
			data.m_screen_quad.draw();
			data.m_fractal_noise.m_attachments[0].generate_mipmap();
		}

		data.m_framebuffer.bind_draw();
		m_program.use();
		data.set_mvp(3.0f, 2.0f, 0.1f, 5.0f);
		m_program.set_uniform("view", glUniformMatrix4fv, 1, GL_TRUE, data.m_view_transposed.data());
		m_program.set_uniform("proj", glUniformMatrix4fv, 1, GL_FALSE, data.m_proj.data());
		m_program.set_uniform("tessellation_amount", glUniform1f, m_tessellation_amount);
		data.m_fractal_noise.m_attachments[0].bind_slot(GL_TEXTURE0);
		glEnable(GL_DEPTH_TEST);
		data.m_isosphere.draw(GL_PATCHES);
	}

	void additionnal_gui() override {
		ImGui::SliderFloat("tessellation amount", &m_tessellation_amount, 1.0f, 50.0f);
	}

	float m_tessellation_amount = 25.0f;
};

struct PipelineGBuffer final : PipelineBase {

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
			data.m_fractal_noise.m_attachments[0].generate_mipmap();
		}

		m_program.use();
		m_gbuffer.resize(data.m_framebuffer.m_w, data.m_framebuffer.m_h);
		m_gbuffer.clear();

		m_gbuffer.bind_draw();
		data.set_mvp(2.0f, 2.0f, 0.8f, 5.0f);
		m_program.set_uniform("view", glUniformMatrix4fv, 1, GL_TRUE, data.m_view_transposed.data());
		m_program.set_uniform("proj", glUniformMatrix4fv, 1, GL_FALSE, data.m_proj.data());
		data.m_fractal_noise.m_attachments[0].bind_slot(GL_TEXTURE0);
		glEnable(GL_DEPTH_TEST);
		data.m_isosphere.draw();

		data.m_framebuffer.blit_from(m_gbuffer, GL_LINEAR, GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(m_mode));
		if (m_mode == Mode::Depth) {
			data.m_framebuffer.m_attachments[0].set_swizzle_mask(GL_RED, GL_RED, GL_RED);
		}
	}

	void additionnal_gui() override
	{
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

struct PipelineCompute final : PipelineBase {
	void draw(LoopData& data) override {
		data.m_scattering_LUT.resize(512, 512);
		if (!m_computed) {
			m_program.use();
			data.m_scattering_LUT.m_attachments[0].bind_as_image_texture();
			glDispatchCompute(data.m_scattering_LUT.m_w / 16, data.m_scattering_LUT.m_h / 16, 1);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			m_computed = true;
		}
		data.m_framebuffer.blit_from(data.m_scattering_LUT);
	}

	bool m_computed = false;
};

struct PipelineAtmosphere final : PipelineBase {
	void draw(LoopData& data) override {
		data.m_pipelines.find("atmosphere scattering")->second->draw(data);

		data.m_framebuffer.clear();
		data.m_framebuffer.bind_draw();
		m_program.use();
		m_program.set_uniform("sun_direction", glUniform3f, m_sun_direction[0], m_sun_direction[1], m_sun_direction[2]);
		data.m_scattering_LUT.m_attachments[0].bind_slot(GL_TEXTURE0);
		glDisable(GL_DEPTH_TEST);
		data.m_screen_quad.draw();
	}

	void additionnal_gui() override
	{
		//ImGui::SliderFloat("theta", &m_theta_deg, 120.0f, 250.0f);
		ImGui::SliderFloat("phi", &m_phi_deg, 0, 180.0f);

		constexpr float pi = 3.14159265f;
		const float t = m_theta_deg * (pi / 180.0f), p = m_phi_deg * (pi / 180.0f);
		const float cost = std::cos(t), sint = std::sin(t), cosp = std::cos(p), sinp = std::sin(p);
		m_sun_direction = v3{ sint * cosp, sint * sinp, cost };
	}

	float m_theta_deg = 145.0f;
	float m_phi_deg = 5.0f;
	v3 m_sun_direction;
};

ShaderSuite get_all_suite()
{
	ShaderSuite suite;

	// Readme examples
	suite.add_shader(ShaderGroup::Readme, ShaderEnum::TypeAndOperatorsExample, types_operators_example);
	suite.add_shader(ShaderGroup::Readme, ShaderEnum::ManualNamingExample, manual_naming_example);
	suite.add_shader(ShaderGroup::Readme, ShaderEnum::AutoNamingExample, auto_naming_example);
	suite.add_shader(ShaderGroup::Readme, ShaderEnum::SwizzlingExample, swizzling_example);
	suite.add_shader(ShaderGroup::Readme, ShaderEnum::QualifiersExample, qualifier_example);
	suite.add_shader(ShaderGroup::Readme, ShaderEnum::ArraysExample, arrays_example);
	suite.add_shader(ShaderGroup::Readme, ShaderEnum::FunctionsExample, functions_example);
	suite.add_shader(ShaderGroup::Readme, ShaderEnum::ControlBlocksExample, control_blocks_example);
	suite.add_shader(ShaderGroup::Readme, ShaderEnum::StructsExample, structs_examples);
	suite.add_shader(ShaderGroup::Readme, ShaderEnum::InterfaceBlocksExamples, interface_examples);
	suite.add_shader(ShaderGroup::Readme, ShaderEnum::CommaAndTypenamesExample, struct_interface_comma_examples);
	suite.add_shader(ShaderGroup::Readme, ShaderEnum::ShaderStageOptionsExample, shader_stage_options);
	suite.add_shader(ShaderGroup::Readme, ShaderEnum::MetaVariation1Example, meta_variation_1);
	suite.add_shader(ShaderGroup::Readme, ShaderEnum::MetaVariation2Example, meta_variation_2);

	suite.add_shader(ShaderGroup::Test, ShaderEnum::ScreenQuadVertex, screen_quad_vertex_shader);

	// Rendering examples 
	suite.add_shader(ShaderGroup::Examples, ShaderEnum::InterfaceVertex, interface_vertex_shader);
	suite.add_shader(ShaderGroup::Examples, ShaderEnum::TexturedMeshFrag, textured_mesh_frag);

	suite.add_shader(ShaderGroup::Examples, ShaderEnum::FractalNoiseFrag, fractal_noise);
	suite.add_shader(ShaderGroup::Examples, ShaderEnum::MultipleOutputsFrag, multiple_outputs_frag);

	suite.add_shader(ShaderGroup::Examples, ShaderEnum::PhongFrag, phong_shading_frag);

	suite.add_shader(ShaderGroup::Examples, ShaderEnum::GeometricNormalsGeom, geometric_normals);
	suite.add_shader(ShaderGroup::Examples, ShaderEnum::SingleColorFrag, single_color_frag);

	suite.add_shader(ShaderGroup::Examples, ShaderEnum::TessControl, tessellation_control_shader_example);
	suite.add_shader(ShaderGroup::Examples, ShaderEnum::TessEval, tessellation_evaluation_shader_example);

	// Rendu examples
	suite.add_shader(ShaderGroup::Rendu, ShaderEnum::AtmosphereScatteringLUT, scattering_lookup_table);
	suite.add_shader(ShaderGroup::Rendu, ShaderEnum::AtmosphereRendering, atmosphere_rendering);

	// Shadertoy examples
	suite.add_shader(ShaderGroup::Shadertoy, ShaderEnum::CSLVaporwave, shader_80);

	// Extra
	suite.add_shader(ShaderGroup::Extra, ShaderEnum::DolphinUbershaderVert, dolphin_ubershader_vertex);
	suite.add_shader(ShaderGroup::Extra, ShaderEnum::DolphinUbershaderFrag, dolphin_ubershader_fragment);

	return suite;
}

std::unordered_map<std::string, std::shared_ptr<PipelineBase>> get_all_pipelines(const LoopData& data)
{
	std::unordered_map<std::string, std::shared_ptr<PipelineBase>> pipelines;
	const auto& shaders = data.m_shader_suite.m_shaders;

	auto pipeline_vaporwave = std::static_pointer_cast<PipelineBase>(std::make_shared<Pipeline80>());
	pipeline_vaporwave->add_shader(GL_VERTEX_SHADER, *shaders.find(ShaderEnum::ScreenQuadVertex));
	pipeline_vaporwave->add_shader(GL_FRAGMENT_SHADER, *shaders.find(ShaderEnum::CSLVaporwave));
	pipelines.emplace("csl_vaporwave", pipeline_vaporwave);

	auto pipeline_textured_mesh = std::static_pointer_cast<PipelineBase>(std::make_shared<PipelineTexturedMesh>());
	pipeline_textured_mesh->add_shader(GL_VERTEX_SHADER, *shaders.find(ShaderEnum::InterfaceVertex));
	pipeline_textured_mesh->add_shader(GL_FRAGMENT_SHADER, *shaders.find(ShaderEnum::TexturedMeshFrag));
	pipelines.emplace("textured_mesh", pipeline_textured_mesh);

	auto pipeline_gbuffer = std::static_pointer_cast<PipelineBase>(std::make_shared<PipelineGBuffer>());
	pipeline_gbuffer->add_shader(GL_VERTEX_SHADER, *shaders.find(ShaderEnum::InterfaceVertex));
	pipeline_gbuffer->add_shader(GL_FRAGMENT_SHADER, *shaders.find(ShaderEnum::MultipleOutputsFrag));
	pipelines.emplace("gbuffer", pipeline_gbuffer);

	auto pipeline_noise = std::static_pointer_cast<PipelineBase>(std::make_shared<PipelineNoise>());
	pipeline_noise->add_shader(GL_VERTEX_SHADER, *shaders.find(ShaderEnum::ScreenQuadVertex));
	pipeline_noise->add_shader(GL_FRAGMENT_SHADER, *shaders.find(ShaderEnum::FractalNoiseFrag));
	pipelines.emplace("noise", pipeline_noise);

	auto pipeline_phong = std::static_pointer_cast<PipelineBase>(std::make_shared<PipelinePhong>());
	pipeline_phong->add_shader(GL_VERTEX_SHADER, *shaders.find(ShaderEnum::InterfaceVertex));
	pipeline_phong->add_shader(GL_FRAGMENT_SHADER, *shaders.find(ShaderEnum::PhongFrag));
	pipelines.emplace("phong", pipeline_phong);

	auto pipeline_geometric_normals = std::static_pointer_cast<PipelineBase>(std::make_shared<PipelineGeometry>());
	pipeline_geometric_normals->add_shader(GL_VERTEX_SHADER, *shaders.find(ShaderEnum::InterfaceVertex));
	pipeline_geometric_normals->add_shader(GL_TESS_CONTROL_SHADER, *shaders.find(ShaderEnum::TessControl));
	pipeline_geometric_normals->add_shader(GL_TESS_EVALUATION_SHADER, *shaders.find(ShaderEnum::TessEval));
	pipeline_geometric_normals->add_shader(GL_GEOMETRY_SHADER, *shaders.find(ShaderEnum::GeometricNormalsGeom));
	pipeline_geometric_normals->add_shader(GL_FRAGMENT_SHADER, *shaders.find(ShaderEnum::SingleColorFrag));
	pipelines.emplace("vertex_normal", pipeline_geometric_normals);

	auto pipeline_displacement = std::static_pointer_cast<PipelineBase>(std::make_shared<PipelineDisplacement>());
	pipeline_displacement->add_shader(GL_VERTEX_SHADER, *shaders.find(ShaderEnum::InterfaceVertex));
	pipeline_displacement->add_shader(GL_TESS_CONTROL_SHADER, *shaders.find(ShaderEnum::TessControl));
	pipeline_displacement->add_shader(GL_TESS_EVALUATION_SHADER, *shaders.find(ShaderEnum::TessEval));
	pipeline_displacement->add_shader(GL_FRAGMENT_SHADER, *shaders.find(ShaderEnum::PhongFrag));
	pipelines.emplace("displacement", pipeline_displacement);

	auto pipeline_compute = std::static_pointer_cast<PipelineBase>(std::make_shared<PipelineCompute>());
	pipeline_compute->add_shader(GL_COMPUTE_SHADER, *shaders.find(ShaderEnum::AtmosphereScatteringLUT));
	pipelines.emplace("atmosphere scattering", pipeline_compute);

	auto pipeline_atmosphere = std::static_pointer_cast<PipelineBase>(std::make_shared<PipelineAtmosphere>());
	pipeline_atmosphere->add_shader(GL_VERTEX_SHADER, *shaders.find(ShaderEnum::ScreenQuadVertex));
	pipeline_atmosphere->add_shader(GL_FRAGMENT_SHADER, *shaders.find(ShaderEnum::AtmosphereRendering));
	pipelines.emplace("atmosphere", pipeline_atmosphere);

	return pipelines;
}

void shader_code_gui(const std::string& code)
{
	if (code.size() < 2000) {
		ImGui::TextWrapped(code.c_str());
	} else {
		ImGui::TextUnformatted(code.data(), code.data() + code.size());
	}
}

enum class Mode : std::size_t
{
	GLSL = 0,
	ImGui = 1,
	Metrics = 3,
	GlobalMetrics = 4
};

template<typename ModeIterator>
void shader_gui(const ModeIterator& mode, ShaderExample& shader, const float vertical_size, const LoopData& data)
{
	ImGui::BeginChild(("text" + shader.m_name).c_str(), ImVec2(0, vertical_size));
	switch (mode.first)
	{
	case Mode::ImGui:
	{
		csl::ImGuiData data;
		shader.m_controller.template print_imgui<csl::Dummy>(data);
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
		s << "\t Shader size : " << shader.m_glsl_str.size() << " characters\n";
		s << "\t Expressions, count : " << mem.m_objects_ids.size() << ", total size : " << mem.get_data_size() << "\n";
		s << "\t Instructions, count : " << ins.m_objects_ids.size() << ", total size : " << ins.get_data_size() << "\n";
		s << "GLSL generation : " << shader.m_glsl_timing << " ms\n";
		ImGui::TextWrapped(s.str().c_str());
		break;
	}
	case Mode::GlobalMetrics:
	{
		const auto& metrics = data.m_global_metrics;
		std::stringstream s;
		s << "Shader suite statistics \n";
		s << "\t Total generation time " << metrics.m_time_total << " ms\n";
		s << "\t Total characters count : " << metrics.m_characters_count << " chars\n";
		s << "\t Memory consumption " << metrics.m_memory << " bytes\n";
		s << "\t Total expressions count " << metrics.m_expressions_count << "\n";
		s << "\t Total instructions count " << metrics.m_instructions_count << "\n";
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
		{ Mode::Metrics, "Metrics"},
		{ Mode::GlobalMetrics, "Global metrics"}
	};

	static const std::unordered_map<ShaderGroup, std::string> shader_group_strs = {
		{ ShaderGroup::Test, "Test"},
		{ ShaderGroup::Readme, "Readme examples"},
		{ ShaderGroup::Examples, "Demos"},
		{ ShaderGroup::Shadertoy, "Shadertoy"},
		{ ShaderGroup::Rendu, "Rendu"},
		{ ShaderGroup::Extra, "Extra"},
	};

	static const std::unordered_map<GLShaderType, std::string> shader_type_strs = {
		{ GLShaderType::Vertex, "Vertex"},
		{ GLShaderType::TessellationControl, "TessControl"},
		{ GLShaderType::TessellationEvaluation, "TessEval"},
		{ GLShaderType::Geometry, "Geometry"},
		{ GLShaderType::Fragment, "Fragment"},
		{ GLShaderType::Compute, "Compute"},
	};

	static ShaderPtr current_shader = {}; // data.m_shader_suite.m_shaders.find(ShaderEnum::DolphinUbershaderVert)->second;

	data.m_time += ImGui::GetIO().DeltaTime;

	data.init_gl_once();

	if (ImGui::GetIO().KeysDownDuration[GLFW_KEY_TAB] == 0.0f) {
		data.m_current_pipeline = {};
		if (current_shader) {
			for (auto shader_it = data.m_shader_suite.m_shaders.begin(); shader_it != data.m_shader_suite.m_shaders.end(); ++shader_it) {
				if (shader_it->second != current_shader) {
					continue;
				}
				if (ImGui::GetIO().KeysDownDuration[GLFW_KEY_LEFT_SHIFT] >= 0.0) {
					if (shader_it == data.m_shader_suite.m_shaders.begin()) {
						auto suite_end_it = data.m_shader_suite.m_shaders.end();
						current_shader = (--suite_end_it)->second;
					} else {
						current_shader = (--shader_it)->second;
					}
				} else {
					++shader_it;
					if (shader_it == data.m_shader_suite.m_shaders.end()) {
						shader_it = data.m_shader_suite.m_shaders.begin();
					}
					current_shader = shader_it->second;
				}
				for (const auto& pipeline : data.m_pipelines) {
					for (const auto& shader : pipeline.second->m_shaders) {
						if (shader.second.m_shader == current_shader) {
							data.m_current_pipeline = pipeline.second;
							break;
						}
					}
				}
				break;
			}
		}
	}

	// Shader suite window
	ImGui::SetNextWindowSize(ImVec2(static_cast<float>((data.m_w_screen - 30) * 2) / 3, static_cast<float>((data.m_h_screen - 20) * 4) / 5), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(10 + static_cast<float>(data.m_w_screen) / 3, 10), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Shader suite")) {

		const float w = ImGui::GetContentRegionAvail().x;
		const float h = ImGui::GetContentRegionAvail().y;

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
					bool selected = false;
					if (current_shader && shader == current_shader) {
						selected = true;
					} else if (data.m_current_pipeline) {
						for (const auto& pipeline_shader : data.m_current_pipeline->m_shaders) {
							if (shader == pipeline_shader.second.m_shader) {
								selected = true;
								break;
							}
						}
					}
					if (ImGui::Selectable(shader->m_name.c_str(), selected)) {
						current_shader = shader;

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

		ImGui::SameLine();

		ImGui::BeginChild("right pane", ImVec2(), false);
		float active_shader_count = 0;
		{
			ImGui::BeginChild("top right pane", ImVec2(0.0f, h / 8.0f), true);
			if (data.m_current_pipeline) {
				int count = 0;
				for (auto& shader : data.m_current_pipeline->m_shaders) {
					if (count > 0) {
						ImGui::SameLine();
						ImGui::Text("->");
						ImGui::SameLine();
					}
					ImGui::Checkbox((shader_type_strs.find(shader.first)->second + "##").c_str(), &shader.second.m_active);
					if (ImGui::BeginPopupContextItem(("shader right click ##" + std::to_string(count)).c_str()))
					{
						if (ImGui::Button("print to console")) {
							std::cout << shader.second.m_shader->m_glsl_str << std::endl;
						}
						ImGui::EndPopup();
					}
					if (shader.second.m_active) {
						++active_shader_count;
					}
					++count;
				}
			} else if (current_shader) {
				ImGui::Text("No associated pipeline");
				if (ImGui::BeginPopupContextItem("shader right click ## single"))
				{
					if (ImGui::Button("print to console")) {
						std::cout << current_shader->m_glsl_str << std::endl;
					}
					ImGui::EndPopup();
				}
				active_shader_count = 1.0f;
			}
			if (data.m_current_pipeline) {
				data.m_current_pipeline->additionnal_gui();
			}
			ImGui::EndChild();

			const float shader_code_height = (7.0f * h / 8.0f - 3.0f * ImGui::GetFrameHeightWithSpacing()) / active_shader_count;
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
								shader_gui(mode, *shader.second.m_shader, shader_code_height, data);
								if (mode.first == Mode::GlobalMetrics) {
									break;
								}
								++count;
							}
						} else if (current_shader) {
							shader_gui(mode, *current_shader, shader_code_height, data);
						}
						ImGui::EndTabItem();
					}
				}
				ImGui::EndTabBar();
			}
			ImGui::EndChild();

			ImGui::EndChild();
		}
	}
	ImGui::End();

	// OpenGL visualisation if selected shader has an associated pipeline
	if (data.m_current_pipeline) {
		ImGui::SetNextWindowSize(ImVec2(static_cast<float>(data.m_w_screen - 30) / 3, static_cast<float>((data.m_h_screen - 20) * 4) / 5), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
		if (ImGui::Begin("OpenGL rendering")) {

			const float w = ImGui::GetContentRegionAvail().x;
			const float h = ImGui::GetContentRegionAvail().y;

			data.m_framebuffer.resize(static_cast<int>(w), static_cast<int>(h));

			// default settings, can be overrided by pipelines
			glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));
			glEnable(GL_DEPTH_TEST);
			data.m_framebuffer.m_attachments[0].set_swizzle_mask();

			data.m_framebuffer.clear();
			data.m_framebuffer.bind_draw();
			data.m_current_pipeline->m_program.use();
			data.m_current_pipeline->draw(data);

			ImGui::Image((ImTextureID)(static_cast<std::size_t>(data.m_framebuffer.m_attachments.front().m_gl)), ImVec2(w, h), ImVec2(0, 1), ImVec2(1, 0));
		}
		ImGui::End();
	}
}

int main()
{
	LoopData data;
	data.m_shader_suite = get_all_suite();

	for (const auto& shader : data.m_shader_suite.m_shaders) {
		const ShaderExample& s = *shader.second;
		data.m_global_metrics.m_time_total += s.m_glsl_timing + s.m_generation_timing;
		data.m_global_metrics.m_characters_count += s.m_glsl_str.size();
		data.m_global_metrics.m_expressions_count += s.m_controller.m_memory_pool.m_objects_ids.size();
		data.m_global_metrics.m_instructions_count += s.m_controller.m_instruction_pool.m_objects_ids.size();
		data.m_global_metrics.m_memory += s.m_controller.m_memory_pool.get_data_size() + s.m_controller.m_instruction_pool.get_data_size();
	}

	// lambda to copy previous frame as texture
	auto before_clear_func = [&] {
		data.m_previous_rendering.resize(data.m_w_screen, data.m_h_screen);
		GLFramebuffer fb_tmp;
		fb_tmp.init_gl();
		fb_tmp.add_attachment(data.m_previous_rendering, GL_DRAW_FRAMEBUFFER);
		fb_tmp.blit_inversed_from(GLFramebuffer::get_default(data.m_w_screen, data.m_h_screen), GL_LINEAR);
		data.m_previous_rendering.generate_mipmap();
	};

	create_context_and_run(
		before_clear_func,
		[&](GLFWwindow* window) {
		glfwGetFramebufferSize(window, &data.m_w_screen, &data.m_h_screen);
		main_loop(data);
	});

	return 0;
}
