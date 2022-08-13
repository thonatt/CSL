#include <array>
#include <cmath>
#include <iostream>
#include <chrono>
#include <filesystem>
#include <map>
#include <memory>

#include "picogl/framework/application.h"
#include "picogl/framework/asset_io.h"

#define PICOGL_IMPLEMENTATION
#include "picogl/picogl.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include <shaders/dolphin.h>
#include <shaders/readme_examples.h>
#include <shaders/rendering.h>
#include <shaders/rendu.h>
#include <shaders/shadertoy.h>

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
		auto start = Clock::now();
		m_shader = std::make_unique<decltype(f())>(f());
		m_generation_timing = std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - start).count() / 1000.0;

		m_glsl_timing = get_timing([&]
			{
				csl::GLSLData glsl_data;
				m_shader->print_glsl(glsl_data);
				m_glsl_str = glsl_data.stream.str();
			});
	}

	std::string m_glsl_str;
	std::unique_ptr<csl::ShaderController> m_shader;
	std::string m_name;
	double m_generation_timing, m_glsl_timing;
};

enum class ShaderGroup
{
	Test,
	Examples,
	Readme,
	Rendu,
	Shadertoy,
	Extra
};

enum class ShaderId
{
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

const std::string& shader_name(const ShaderId shader)
{
	static const std::unordered_map<ShaderId, std::string> shaders_strs = {
		{ ShaderId::TypeAndOperatorsExample, "Types & Operators" },
		{ ShaderId::ManualNamingExample, "Manual naming" },
		{ ShaderId::AutoNamingExample, "Automatic naming" },
		{ ShaderId::SwizzlingExample, "Swizzling" },
		{ ShaderId::QualifiersExample, "Qualifiers" },
		{ ShaderId::ArraysExample, "Arrays" },
		{ ShaderId::FunctionsExample, "Functions" },
		{ ShaderId::ControlBlocksExample, "Control blocks" },
		{ ShaderId::StructsExample, "Structs" },
		{ ShaderId::InterfaceBlocksExamples, "Interface blocks" },
		{ ShaderId::CommaAndTypenamesExample, "Comma & typenames" },
		{ ShaderId::ShaderStageOptionsExample, "Shader stage options" },
		{ ShaderId::MetaVariation1Example, "Variation 1" },
		{ ShaderId::MetaVariation2Example, "Variation 2" },
		{ ShaderId::ScreenQuadVertex, "Screen quad" },
		{ ShaderId::InterfaceVertex, "Interface vert" },
		{ ShaderId::TexturedMeshFrag, "Textured mesh frag" },
		{ ShaderId::FractalNoiseFrag, "Fractal noise frag" },
		{ ShaderId::MultipleOutputsFrag, "Multiple outputs frag" },
		{ ShaderId::PhongFrag, "Phong shading frag" },
		{ ShaderId::GeometricNormalsGeom, "Geometric normals geom" },
		{ ShaderId::SingleColorFrag, "Single color frag" },
		{ ShaderId::TessControl, "Tessellation control" },
		{ ShaderId::TessEval, "Tessellation evaluation" },
		{ ShaderId::AtmosphereScatteringLUT, "Atmosphere LUT compute" },
		{ ShaderId::AtmosphereRendering, "Atmosphere scattering" },
		{ ShaderId::CSLVaporwave, "Vaporwave CSL" },
		{ ShaderId::DolphinUbershaderVert, "Dolphin Ubershader vert" },
		{ ShaderId::DolphinUbershaderFrag, "Dolphin Ubershader frag" },
	};

	return shaders_strs.find(shader)->second;
}

struct ShaderSuite
{
	template<typename F>
	void add_shader(const ShaderGroup group, const ShaderId shader, F&& f) {
		auto shader_example = std::make_shared<ShaderExample>(std::forward<F>(f));
		shader_example->m_name = shader_name(shader);
		m_shaders.emplace(shader, shader_example);
		m_groups[group].emplace_back(shader_example);
	}

	std::map<ShaderId, std::shared_ptr<ShaderExample>> m_shaders;
	std::unordered_map<ShaderGroup, std::vector<std::shared_ptr<ShaderExample>>> m_groups;
};

struct Application;

struct PipelineBase
{
	struct ShaderActive
	{
		std::shared_ptr<ShaderExample> m_shader;
		bool m_active = true;
	};

	void add_shader(const GLenum type, const std::pair<ShaderId, std::shared_ptr<ShaderExample>>& shader)
	{
		m_shaders[type] = { shader.second };
	}

	void compile()
	{
		std::vector<picogl::Shader> shaders(m_shaders.size());
		int shader_count = 0;
		for (const auto& [type, shader] : m_shaders)
			shaders[shader_count++] = picogl::Shader::make(type, shader.m_shader->m_glsl_str);
		m_program = picogl::Program::make({ shaders.begin(), shaders.end() });
	}

	virtual void additionnal_gui() {};
	virtual void draw(Application& data) {};
	virtual ~PipelineBase() = default;

	picogl::Program m_program;
	std::unordered_map<GLenum, ShaderActive> m_shaders;
};

struct GlobalMetrics
{
	double m_time_total = 0.0f;
	std::size_t m_characters_count = 0;
	std::size_t m_expressions_count = 0;
	std::size_t m_instructions_count = 0;
	std::size_t m_memory = 0;
};

ShaderSuite get_all_suite()
{
	ShaderSuite suite;

	// Readme examples.
	suite.add_shader(ShaderGroup::Readme, ShaderId::TypeAndOperatorsExample, types_operators_example);
	suite.add_shader(ShaderGroup::Readme, ShaderId::ManualNamingExample, manual_naming_example);
	suite.add_shader(ShaderGroup::Readme, ShaderId::AutoNamingExample, auto_naming_example);
	suite.add_shader(ShaderGroup::Readme, ShaderId::SwizzlingExample, swizzling_example);
	suite.add_shader(ShaderGroup::Readme, ShaderId::QualifiersExample, qualifier_example);
	suite.add_shader(ShaderGroup::Readme, ShaderId::ArraysExample, arrays_example);
	suite.add_shader(ShaderGroup::Readme, ShaderId::FunctionsExample, functions_example);
	suite.add_shader(ShaderGroup::Readme, ShaderId::ControlBlocksExample, control_blocks_example);
	suite.add_shader(ShaderGroup::Readme, ShaderId::StructsExample, structs_examples);
	suite.add_shader(ShaderGroup::Readme, ShaderId::InterfaceBlocksExamples, interface_examples);
	suite.add_shader(ShaderGroup::Readme, ShaderId::CommaAndTypenamesExample, struct_interface_comma_examples);
	suite.add_shader(ShaderGroup::Readme, ShaderId::ShaderStageOptionsExample, shader_stage_options);
	suite.add_shader(ShaderGroup::Readme, ShaderId::MetaVariation1Example, meta_variation_1);
	suite.add_shader(ShaderGroup::Readme, ShaderId::MetaVariation2Example, meta_variation_2);

	suite.add_shader(ShaderGroup::Test, ShaderId::ScreenQuadVertex, screen_quad_vertex_shader);

	// Rendering examples.
	suite.add_shader(ShaderGroup::Examples, ShaderId::InterfaceVertex, interface_vertex_shader);
	suite.add_shader(ShaderGroup::Examples, ShaderId::TexturedMeshFrag, textured_mesh_frag);

	suite.add_shader(ShaderGroup::Examples, ShaderId::FractalNoiseFrag, fractal_noise);
	suite.add_shader(ShaderGroup::Examples, ShaderId::MultipleOutputsFrag, multiple_outputs_frag);

	suite.add_shader(ShaderGroup::Examples, ShaderId::PhongFrag, phong_shading_frag);

	suite.add_shader(ShaderGroup::Examples, ShaderId::GeometricNormalsGeom, geometric_normals);
	suite.add_shader(ShaderGroup::Examples, ShaderId::SingleColorFrag, single_color_frag);

	suite.add_shader(ShaderGroup::Examples, ShaderId::TessControl, tessellation_control_shader_example);
	suite.add_shader(ShaderGroup::Examples, ShaderId::TessEval, tessellation_evaluation_shader_example);

	// Rendu examples.
	suite.add_shader(ShaderGroup::Rendu, ShaderId::AtmosphereScatteringLUT, scattering_lookup_table);
	suite.add_shader(ShaderGroup::Rendu, ShaderId::AtmosphereRendering, atmosphere_rendering);

	// Shadertoy examples.
	suite.add_shader(ShaderGroup::Shadertoy, ShaderId::CSLVaporwave, shader_80);

	// Extra.
	suite.add_shader(ShaderGroup::Extra, ShaderId::DolphinUbershaderVert, dolphin_ubershader_vertex);
	suite.add_shader(ShaderGroup::Extra, ShaderId::DolphinUbershaderFrag, dolphin_ubershader_fragment);

	return suite;
}

std::unordered_map<std::string, std::shared_ptr<PipelineBase>> get_all_pipelines(const Application& data);

void shader_code_gui(const std::string& code)
{
	if (code.size() < 2000)
		ImGui::TextWrapped(code.c_str());
	else
		ImGui::TextUnformatted(code.data(), code.data() + code.size());
}

enum class Mode : std::size_t
{
	GLSL = 0,
	ImGui = 1,
	Metrics = 3,
	GlobalMetrics = 4
};

template<typename ModeIterator>
void shader_gui(const ModeIterator& mode, ShaderExample& shader, const float vertical_size, const GlobalMetrics& metrics)
{
	ImGui::BeginChild(("text" + shader.m_name).c_str(), ImVec2(0, vertical_size));
	switch (mode.first)
	{
	case Mode::ImGui:
	{
		csl::ImGuiData data;
		shader.m_shader->print_imgui(data);
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
		const auto& mem = shader.m_shader->m_memory_pool;
		const auto& ins = shader.m_shader->m_instruction_pool;
		s << "\t Shader size : " << shader.m_glsl_str.size() << " characters\n";
		s << "\t Expressions, count : " << mem.m_objects_offsets.size() << ", total size : " << mem.get_data_size() << "\n";
		s << "\t Instructions, count : " << ins.m_objects_offsets.size() << ", total size : " << ins.get_data_size() << "\n";
		s << "GLSL generation : " << shader.m_glsl_timing << " ms\n";
		ImGui::TextWrapped(s.str().c_str());
		break;
	}
	case Mode::GlobalMetrics:
	{
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

struct Application : public framework::Application
{
	Application() : framework::Application("CSL shader suite") { }

	void gui() override {}
	void render() override {}

	void setup() override
	{
		framework::Application::setup();

		std::filesystem::path path = std::string(CSL_SHADER_SUITE_RESOURCE_PATH) + "/shadertoy-font-25.png";
		m_tex_letters = framework::make_texture_from_file(std::filesystem::absolute(path).string());

		m_screen_quad = picogl::Mesh::make();
		m_screen_quad.set_indices(GL_TRIANGLES, std::vector<glm::u32>{ 0, 1, 2, 0, 3, 2 });
		m_screen_quad.set_vertex_attributes({
				{ std::vector<glm::vec3>{ { 0,0,0 }, { 1,0,0 }, { 1,1,0 }, { 0,1,0 } }, GL_FLOAT, 3},
				{ std::vector<glm::vec2>{ { 0,0 }, { 1,0 }, { 1,1 }, { 0,1 } }, GL_FLOAT, 2 }
			});

		m_triangle = picogl::Mesh::make();
		m_triangle.set_indices(GL_TRIANGLES, std::vector<glm::u32>{ 0, 1, 2 });
		m_triangle.set_vertex_attributes({
				{ std::vector<glm::vec3>{ { -1,-1,0 }, { -1,1,0 }, { 2,0,0 } }, GL_FLOAT, 3},
				{ std::vector<glm::vec2>{ { 0,0 }, { 1,0 }, { 1,1 }  }, GL_FLOAT, 2 }
			});

		m_quad = picogl::Mesh::make();
		m_quad.set_indices(GL_TRIANGLES, std::vector<glm::u32>{ 0, 1, 2, 0, 3, 2 });
		m_quad.set_vertex_attributes({
				{ std::vector<glm::vec3>{ { -1,-1,0 }, { -1,1,0 }, { 1,1,0 }, { 1,-1,0 } }, GL_FLOAT, 3},
				{ std::vector<glm::vec2>{ { 0,0 }, { 0,1 }, { 1,1 }, { 1,0 }  }, GL_FLOAT, 2 }
			});

		const float t = (1.0f + std::sqrt(5.0f)) / 2.0f;
		constexpr float pi = 3.14159265f;

		m_isosphere = picogl::Mesh::make();
		m_isosphere.set_indices(GL_TRIANGLES, std::vector<glm::u32>{ 0, 11, 5, 0, 5, 1, 0, 1, 7, 0, 7, 10, 0, 10, 11, 1, 5, 9, 5, 11, 4, 11, 10, 2, 10, 7, 6, 7, 1, 8,
			3, 9, 4, 3, 4, 2, 3, 2, 6, 3, 6, 8, 3, 8, 9, 4, 9, 5, 2, 4, 11, 6, 2, 10, 8, 6, 7, 9, 8, 1 });

		const std::vector<glm::vec3> ps = {
			{-1, t, 0}, {1, t, 0}, {-1, -t, 0}, {1, -t, 0},
			{0, -1, t}, {0, 1, t}, {0, -1, -t}, {0, 1, -t},
			{t, 0, -1}, {t, 0, 1}, {-t, 0, -1}, {-t, 0, 1}
		};

		std::vector<glm::vec3> ns(ps.size());
		std::vector<glm::vec2> uvs(ps.size());
		for (std::size_t i = 0; i < ps.size(); ++i) {
			ns[i] = glm::normalize(ps[i]);
			uvs[i][0] = std::acos(ns[i][2]) / pi;
			uvs[i][1] = 0.5f * (1.0f + std::atan2(ns[i][1], ns[i][0]) / pi);
		}
		m_isosphere.set_vertex_attributes({
				{ps, GL_FLOAT, 3},
				{uvs, GL_FLOAT, 2},
				{ns, GL_FLOAT, 3}
			});

		for (int i = 0; i < 10; ++i)
			m_shader_suite = get_all_suite();

		for (const auto& [typen, shader] : m_shader_suite.m_shaders) {
			m_global_metrics.m_time_total += shader->m_glsl_timing + shader->m_generation_timing;
			m_global_metrics.m_characters_count += shader->m_glsl_str.size();
			m_global_metrics.m_expressions_count += shader->m_shader->m_memory_pool.m_objects_offsets.size();
			m_global_metrics.m_instructions_count += shader->m_shader->m_instruction_pool.m_objects_offsets.size();
			m_global_metrics.m_memory += shader->m_shader->m_memory_pool.get_data_size() + shader->m_shader->m_instruction_pool.get_data_size();
		}

		m_pipelines = get_all_pipelines(*this);
		m_current_pipeline = m_pipelines.find("csl_vaporwave")->second;
		m_current_shader = m_shader_suite.m_shaders.at(ShaderId::CSLVaporwave);
	}

	void update() override {
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

		static const std::unordered_map<GLenum, std::string> shader_type_strs = {
			{ GL_VERTEX_SHADER, "Vertex"},
			{ GL_TESS_CONTROL_SHADER, "TessControl"},
			{ GL_TESS_EVALUATION_SHADER, "TessEval"},
			{ GL_GEOMETRY_SHADER, "Geometry"},
			{ GL_FRAGMENT_SHADER, "Fragment"},
			{ GL_COMPUTE_SHADER, "Compute"},
		};

		glfwGetFramebufferSize(m_main_window.get(), &m_w_screen, &m_h_screen);
		if (m_w_screen == 0 || m_h_screen == 0)
			return;

		if (m_previous_rendering.width() != m_w_screen || m_previous_rendering.height() != m_h_screen) {
			m_previous_rendering = picogl::Framebuffer::make(m_w_screen, m_h_screen);
			m_previous_rendering.add_color_attachment(GL_RGBA8, GL_TEXTURE_2D, picogl::Texture::Options::AllocateMipmap);
		}

		// Copy previous frame as (inversed) texture.
		picogl::Framebuffer::get_default(m_w_screen, m_h_screen).blit_to(
			m_previous_rendering, 0, m_h_screen, m_w_screen, 0,
			GL_COLOR_ATTACHMENT0, GL_LINEAR,
			0, 0, m_w_screen, m_h_screen, GL_BACK);

		m_previous_rendering.color_attachments()[0].generate_mipmap();
		picogl::Framebuffer::get_default(m_w_screen, m_h_screen).clear();

		m_time += ImGui::GetIO().DeltaTime;

		if (m_current_shader && (ImGui::IsKeyPressed(GLFW_KEY_TAB) || ImGui::IsKeyPressed(GLFW_KEY_LEFT) || ImGui::IsKeyPressed(GLFW_KEY_RIGHT))) {
			m_current_pipeline = {};
			for (auto shader_it = m_shader_suite.m_shaders.begin(); shader_it != m_shader_suite.m_shaders.end(); ++shader_it) {
				if (shader_it->second != m_current_shader)
					continue;

				if (ImGui::IsKeyDown(GLFW_KEY_LEFT_SHIFT) || ImGui::IsKeyPressed(GLFW_KEY_LEFT)) {
					if (shader_it == m_shader_suite.m_shaders.begin()) {
						auto suite_end_it = m_shader_suite.m_shaders.end();
						m_current_shader = (--suite_end_it)->second;
					} else {
						m_current_shader = (--shader_it)->second;
					}
				} else  if (ImGui::IsKeyPressed(GLFW_KEY_TAB) || ImGui::IsKeyPressed(GLFW_KEY_RIGHT)) {
					++shader_it;
					if (shader_it == m_shader_suite.m_shaders.end())
						shader_it = m_shader_suite.m_shaders.begin();
					m_current_shader = shader_it->second;
				}
				for (const auto& [name, pipeline] : m_pipelines)
					for (const auto& shader : pipeline->m_shaders)
						if (shader.second.m_shader == m_current_shader) {
							m_current_pipeline = pipeline;
							break;
						}
				break;
			}
		}

		// Shader suite window
		ImGui::SetNextWindowSize(ImVec2(static_cast<float>((m_w_screen - 30) * 2) / 3, static_cast<float>((m_h_screen - 20) * 5) / 5));
		ImGui::SetNextWindowPos(ImVec2(15 + static_cast<float>(m_w_screen - 30) / 3, 10));
		if (ImGui::Begin("Shader suite", nullptr, ImGuiWindowFlags_NoMove))
		{
			const float w = ImGui::GetContentRegionAvail().x;
			const float h = ImGui::GetContentRegionAvail().y;

			ImGui::BeginChild("left pane", ImVec2(w / 4, 0), true);
			for (const auto& [group_type, group] : m_shader_suite.m_groups)
			{
				if (group_type == ShaderGroup::Test)
					continue;

				ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
				if (ImGui::TreeNode(shader_group_strs.find(group_type)->second.c_str()))
				{
					for (const auto& shader : group) {
						bool selected = false;
						if (m_current_shader && shader == m_current_shader)
							selected = true;
						else if (m_current_pipeline)
							for (const auto& pipeline_shader : m_current_pipeline->m_shaders)
								if (shader == pipeline_shader.second.m_shader) {
									selected = true;
									break;
								}

						if (ImGui::Selectable(shader->m_name.c_str(), selected)) {
							m_current_shader = shader;

							bool pipeline_found = false;
							for (const auto& [name, pipeline] : m_pipelines) {
								for (const auto& [type, shader] : pipeline->m_shaders)
									if (m_current_shader->m_name == shader.m_shader->m_name) {
										m_current_pipeline = pipeline;
										pipeline_found = true;
									}
							}

							if (!pipeline_found)
								m_current_pipeline = {};
						}
					}
					ImGui::TreePop();
				}
			}
			ImGui::EndChild();

			ImGui::SameLine();

			if (ImGui::BeginChild("right pane", ImVec2(), false)) {
				float active_shader_count = 0;

				if (ImGui::BeginChild("top right pane", ImVec2(0.0f, h / 8.0f), true))
				{
					int count = 0;
					auto print_shader = [&](const ShaderExample& shader)
					{
						if (ImGui::BeginPopupContextItem(("shader right click ##" + std::to_string(count)).c_str()))
						{
							if (ImGui::Button("print to console"))
								std::cout << shader.m_glsl_str << std::endl;
							ImGui::EndPopup();
						}
					};

					if (m_current_pipeline) {

						for (auto& [type, shader] : m_current_pipeline->m_shaders) {
							if (count > 0) {
								ImGui::SameLine();
								ImGui::Text("->");
								ImGui::SameLine();
							}
							ImGui::Checkbox((shader_type_strs.find(type)->second + "##").c_str(), &shader.m_active);
							print_shader(*shader.m_shader);
							if (shader.m_active)
								++active_shader_count;
							++count;
						}
					} else if (m_current_shader) {
						ImGui::Text("No associated pipeline.");
						print_shader(*m_current_shader);
						active_shader_count = 1.0f;
					}
					if (m_current_pipeline)
						m_current_pipeline->additionnal_gui();
				}
				ImGui::EndChild();

				const float shader_code_height = (7.0f * h / 8.0f - 3.0f * ImGui::GetFrameHeightWithSpacing()) / active_shader_count;
				if (ImGui::BeginChild("bottom right pane", ImVec2(), true)) {
					if (ImGui::BeginTabBar("mode_bar", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs)) {
						for (const auto& mode : mode_strs)
							if (ImGui::BeginTabItem(mode.second.c_str())) {
								if (m_current_pipeline) {
									int count = 0;
									for (auto& [type, shader] : m_current_pipeline->m_shaders) {
										if (!shader.m_active)
											continue;
										if (count)
											ImGui::Separator();
										shader_gui(mode, *shader.m_shader, shader_code_height, m_global_metrics);
										if (mode.first == Mode::GlobalMetrics)
											break;
										++count;
									}
								} else if (m_current_shader)
									shader_gui(mode, *m_current_shader, shader_code_height, m_global_metrics);
								ImGui::EndTabItem();
							}
						ImGui::EndTabBar();
					}
				}
				ImGui::EndChild();
			}
			ImGui::EndChild();
		}
		ImGui::End();

		// OpenGL visualisation if selected shader has an associated pipeline.
		if (m_current_pipeline) {
			ImGui::SetNextWindowSize(ImVec2(static_cast<float>(m_w_screen - 30) / 3, static_cast<float>((m_h_screen - 20) * 5) / 5));
			ImGui::SetNextWindowPos(ImVec2(10, 10));
			if (ImGui::Begin("OpenGL rendering", nullptr, ImGuiWindowFlags_NoMove)) {

				const float w = ImGui::GetContentRegionAvail().x;
				const float h = ImGui::GetContentRegionAvail().y;

				if (m_framebuffer.width() != w || m_framebuffer.height() != h) {
					m_framebuffer = picogl::Framebuffer::make(static_cast<GLsizei>(w), static_cast<GLsizei>(h));
					m_framebuffer.add_color_attachment(GL_RGBA8);
					m_framebuffer.set_depth_attachment();
				}

				glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));
				glEnable(GL_DEPTH_TEST);
				const_cast<picogl::Texture&>(m_framebuffer.color_attachments()[0]).set_swizzling();

				m_framebuffer.clear();
				m_framebuffer.bind_draw();
				m_current_pipeline->m_program.use();
				m_current_pipeline->draw(*this);
				ImGui::Image((ImTextureID)(static_cast<std::size_t>(m_framebuffer.color_attachments()[0])), ImVec2(w, h), ImVec2(0, 1), ImVec2(1, 0));
			}
			ImGui::End();
		}
	}

	void set_mvp(const float distance, const float radius, const float near, const float far)
	{
		constexpr float two_pi = 2.0f * 3.141519265f;
		const glm::vec3 at = { 0, 0, 0 };
		const glm::vec3 up = { 0, 0, 1 };
		const float theta = std::fmod(m_time / 12.0f, 1.0f) * two_pi;
		m_eye = glm::vec3{ radius * std::cos(theta), radius * std::sin(theta), distance };

		m_view = glm::lookAt(m_eye, at, up);
		m_proj = glm::perspective(glm::radians(90.0f), 0.7f, near, 2.0f * far);
	}

	void render_noise_texture()
	{
		if (m_fractal_noise.width() != m_framebuffer.width() || m_fractal_noise.height() != m_framebuffer.height())
		{
			m_fractal_noise = picogl::Framebuffer::make(m_framebuffer.width(), m_framebuffer.height());
			m_fractal_noise.add_color_attachment(GL_RGBA8);
		}

		m_pipelines.find("noise")->second->m_program.use();
		m_fractal_noise.clear();
		m_fractal_noise.bind_draw();
		glDisable(GL_DEPTH_TEST);
		m_screen_quad.draw();
		m_fractal_noise.color_attachments()[0].generate_mipmap();
	}

	ShaderSuite m_shader_suite;
	std::unordered_map<std::string, std::shared_ptr<PipelineBase>> m_pipelines;
	std::shared_ptr<PipelineBase> m_current_pipeline = {};
	std::shared_ptr<ShaderExample> m_current_shader = {};
	picogl::Texture m_tex_letters;
	picogl::Framebuffer m_framebuffer, m_previous_rendering, m_fractal_noise, m_scattering_LUT;
	picogl::Mesh m_isosphere, m_quad, m_screen_quad, m_triangle;
	GlobalMetrics m_global_metrics;
	glm::mat4 m_view, m_proj;
	glm::vec3 m_eye;
	float m_time = 0.0f;
	int m_w_screen = 0, m_h_screen = 0;
};

struct PipelineGrid final : PipelineBase
{
	void draw(Application& data) override {
		m_program.set_uniform("time", glUniform1f, data.m_time);
		glDisable(GL_DEPTH_TEST);
		data.m_screen_quad.draw();
	}
};

struct Pipeline80 final : PipelineBase
{
	void draw(Application& data) override {
		m_program.set_uniform("iTime", glUniform1f, data.m_time);
		m_program.set_uniform("iResolution", glUniform2f, (float)data.m_framebuffer.width(), (float)data.m_framebuffer.height());
		data.m_tex_letters.bind_as_sampler(GL_TEXTURE0);
		glDisable(GL_DEPTH_TEST);
		data.m_screen_quad.draw();
	}
};

struct PipelineTexturedMesh final : PipelineBase
{
	void draw(Application& data) override {
		data.m_framebuffer.clear({ 0.3f, 0.3f, 0.3f, 1.0f });
		data.set_mvp(1.2f, 0.1f, 0.1f, 5.0f);
		m_program.set_uniform("view", glUniformMatrix4fv, 1, GL_FALSE, glm::value_ptr(data.m_view));
		m_program.set_uniform("proj", glUniformMatrix4fv, 1, GL_FALSE, glm::value_ptr(data.m_proj));
		data.m_previous_rendering.color_attachments()[0].bind_as_sampler(GL_TEXTURE0);
		data.m_quad.draw();
	}
};

struct PipelineNoise final : PipelineBase
{
	void draw(Application& data) override {
		if (data.m_fractal_noise.width() != data.m_framebuffer.width() || data.m_fractal_noise.height() != data.m_framebuffer.height())
		{
			data.m_fractal_noise = picogl::Framebuffer::make(data.m_framebuffer.width(), data.m_framebuffer.height());
			data.m_fractal_noise.add_color_attachment(GL_RGBA8);
		}

		data.m_fractal_noise.bind_draw();
		m_program.set_uniform("freq_count", glUniform1i, m_frequencies_count);
		m_program.set_uniform("uv_scaling", glUniform1f, m_uv_scaling);
		glDisable(GL_DEPTH_TEST);
		data.m_screen_quad.draw();
		data.m_fractal_noise.blit_to(data.m_framebuffer);
	}

	void additionnal_gui() override
	{
		ImGui::SliderInt("frequencies count", &m_frequencies_count, 1, 8);
		ImGui::SliderFloat("uv scaling", &m_uv_scaling, 2.0, 32.0);
	}

	int m_frequencies_count = 3;
	float m_uv_scaling = 8.0f;
};

struct PipelinePhong final : PipelineBase
{
	void draw(Application& data) override {
		data.set_mvp(2.5f, 2.0f, 0.8f, 5.0f);
		m_program.set_uniform("view", glUniformMatrix4fv, 1, GL_FALSE, glm::value_ptr(data.m_view));
		m_program.set_uniform("proj", glUniformMatrix4fv, 1, GL_FALSE, glm::value_ptr(data.m_proj));
		m_program.set_uniform("eye", glUniform3f, data.m_eye[0], data.m_eye[1], data.m_eye[2]);
		data.m_isosphere.draw();
	}
};

struct PipelineGeometry final : PipelineBase
{
	void draw(Application& data) override
	{
		data.render_noise_texture();

		data.m_framebuffer.bind_draw();
		data.set_mvp(3.0f, 2.0f, 0.1f, 5.0f);
		glEnable(GL_DEPTH_TEST);

		// Render tessellated mesh.
		{
			auto& displacement_pipeline = data.m_pipelines.find("displacement")->second->m_program;
			displacement_pipeline.use();
			displacement_pipeline.set_uniform("view", glUniformMatrix4fv, 1, GL_FALSE, glm::value_ptr(data.m_view));
			displacement_pipeline.set_uniform("proj", glUniformMatrix4fv, 1, GL_FALSE, glm::value_ptr(data.m_proj));
			displacement_pipeline.set_uniform("tessellation_amount", glUniform1f, 100.0f);
			data.m_fractal_noise.color_attachments()[0].bind_as_sampler(GL_TEXTURE0);
			data.m_isosphere.draw(GL_PATCHES);
		}

		data.m_framebuffer.bind_draw();
		m_program.use();
		m_program.set_uniform("view", glUniformMatrix4fv, 1, GL_FALSE, glm::value_ptr(data.m_view));
		m_program.set_uniform("proj", glUniformMatrix4fv, 1, GL_FALSE, glm::value_ptr(data.m_proj));
		m_program.set_uniform("tessellation_amount", glUniform1f, 100.0f);
		data.m_fractal_noise.color_attachments()[0].bind_as_sampler(GL_TEXTURE0);
		data.m_isosphere.draw(GL_PATCHES);
	}
};

struct PipelineDisplacement final : PipelineBase
{
	void draw(Application& data) override
	{
		data.render_noise_texture();

		data.m_framebuffer.bind_draw();
		m_program.use();
		data.set_mvp(3.0f, 2.0f, 0.1f, 5.0f);
		m_program.set_uniform("view", glUniformMatrix4fv, 1, GL_FALSE, glm::value_ptr(data.m_view));
		m_program.set_uniform("proj", glUniformMatrix4fv, 1, GL_FALSE, glm::value_ptr(data.m_proj));
		m_program.set_uniform("tessellation_amount", glUniform1f, m_tessellation_amount);
		data.m_fractal_noise.color_attachments()[0].bind_as_sampler(GL_TEXTURE0);
		glEnable(GL_DEPTH_TEST);
		data.m_isosphere.draw(GL_PATCHES);
	}

	void additionnal_gui() override {
		ImGui::SliderFloat("tessellation amount", &m_tessellation_amount, 1.0f, 50.0f);
	}

	float m_tessellation_amount = 25.0f;
};

struct PipelineGBuffer final : PipelineBase
{
	enum class Mode : GLenum {
		Position = 0,
		Normal = 1,
		UV = 2,
		Depth = 3,
		Texture = 4
	};

	void draw(Application& data) override
	{
		if (m_gbuffer.width() != data.m_framebuffer.width() || m_gbuffer.height() != data.m_framebuffer.height())
		{
			m_gbuffer = picogl::Framebuffer::make(data.m_framebuffer.width(), data.m_framebuffer.height());
			m_gbuffer.set_depth_attachment();
			m_gbuffer.add_color_attachment(GL_RGB32F);
			m_gbuffer.add_color_attachment(GL_RGB32F);
			m_gbuffer.add_color_attachment(GL_RG32F);
			m_gbuffer.add_color_attachment(GL_R32F);
			m_gbuffer.add_color_attachment(GL_RGB8);
		}

		data.render_noise_texture();

		m_program.use();
		m_gbuffer.clear();

		m_gbuffer.bind_draw();
		data.set_mvp(3.0f, 2.0f, 0.8f, 5.0f);
		m_program.set_uniform("view", glUniformMatrix4fv, 1, GL_FALSE, glm::value_ptr(data.m_view));
		m_program.set_uniform("proj", glUniformMatrix4fv, 1, GL_FALSE, glm::value_ptr(data.m_proj));
		m_program.set_uniform("tessellation_amount", glUniform1f, 100.0f);
		data.m_fractal_noise.color_attachments()[0].bind_as_sampler(GL_TEXTURE0);
		glEnable(GL_DEPTH_TEST);
		data.m_isosphere.draw(GL_PATCHES);

		m_gbuffer.blit_to(data.m_framebuffer, GL_COLOR_ATTACHMENT0, GL_LINEAR, GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(m_mode));
		if (m_mode == Mode::Depth)
			const_cast<picogl::Texture&>(data.m_framebuffer.color_attachments()[0]).set_swizzling({ GL_RED, GL_RED, GL_RED, GL_ALPHA });
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
			if (ImGui::RadioButton(mode.second.c_str(), mode.first == m_mode))
				m_mode = mode.first;
		}
	}

	picogl::Framebuffer m_gbuffer;
	Mode m_mode = Mode::Depth;
};

struct PipelineCompute final : PipelineBase
{
	void draw(Application& data) override
	{
		if (!m_computed) {
			data.m_scattering_LUT = picogl::Framebuffer::make(512, 512);
			data.m_scattering_LUT.add_color_attachment(GL_RGBA32F);
			m_program.use();
			data.m_scattering_LUT.color_attachments()[0].bind_as_image(0);
			glDispatchCompute(data.m_scattering_LUT.width() / 16, data.m_scattering_LUT.height() / 16, 1);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			m_computed = true;
		}
		data.m_scattering_LUT.blit_to(data.m_framebuffer);
	}

	bool m_computed = false;
};

struct PipelineAtmosphere final : PipelineBase {
	void draw(Application& data) override {
		data.m_pipelines.find("atmosphere scattering")->second->draw(data);

		data.m_framebuffer.clear();
		data.m_framebuffer.bind_draw();
		m_program.use();
		m_program.set_uniform("sun_direction", glUniform3f, m_sun_direction[0], m_sun_direction[1], m_sun_direction[2]);
		data.m_scattering_LUT.color_attachments()[0].bind_as_sampler(GL_TEXTURE0);
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
		m_sun_direction = glm::vec3{ sint * cosp, sint * sinp, cost };
	}

	float m_theta_deg = 145.0f;
	float m_phi_deg = 5.0f;
	glm::vec3 m_sun_direction;
};

std::unordered_map<std::string, std::shared_ptr<PipelineBase>> get_all_pipelines(const Application& data)
{
	std::unordered_map<std::string, std::shared_ptr<PipelineBase>> pipelines;
	const auto& shaders = data.m_shader_suite.m_shaders;

	auto pipeline_vaporwave = std::static_pointer_cast<PipelineBase>(std::make_shared<Pipeline80>());
	pipeline_vaporwave->add_shader(GL_VERTEX_SHADER, *shaders.find(ShaderId::ScreenQuadVertex));
	pipeline_vaporwave->add_shader(GL_FRAGMENT_SHADER, *shaders.find(ShaderId::CSLVaporwave));
	pipelines.emplace("csl_vaporwave", pipeline_vaporwave);

	auto pipeline_textured_mesh = std::static_pointer_cast<PipelineBase>(std::make_shared<PipelineTexturedMesh>());
	pipeline_textured_mesh->add_shader(GL_VERTEX_SHADER, *shaders.find(ShaderId::InterfaceVertex));
	pipeline_textured_mesh->add_shader(GL_FRAGMENT_SHADER, *shaders.find(ShaderId::TexturedMeshFrag));
	pipelines.emplace("textured_mesh", pipeline_textured_mesh);

	auto pipeline_gbuffer = std::static_pointer_cast<PipelineBase>(std::make_shared<PipelineGBuffer>());
	pipeline_gbuffer->add_shader(GL_VERTEX_SHADER, *shaders.find(ShaderId::InterfaceVertex));
	pipeline_gbuffer->add_shader(GL_TESS_CONTROL_SHADER, *shaders.find(ShaderId::TessControl));
	pipeline_gbuffer->add_shader(GL_TESS_EVALUATION_SHADER, *shaders.find(ShaderId::TessEval));
	pipeline_gbuffer->add_shader(GL_FRAGMENT_SHADER, *shaders.find(ShaderId::MultipleOutputsFrag));
	pipelines.emplace("gbuffer", pipeline_gbuffer);

	auto pipeline_noise = std::static_pointer_cast<PipelineBase>(std::make_shared<PipelineNoise>());
	pipeline_noise->add_shader(GL_VERTEX_SHADER, *shaders.find(ShaderId::ScreenQuadVertex));
	pipeline_noise->add_shader(GL_FRAGMENT_SHADER, *shaders.find(ShaderId::FractalNoiseFrag));
	pipelines.emplace("noise", pipeline_noise);

	auto pipeline_phong = std::static_pointer_cast<PipelineBase>(std::make_shared<PipelinePhong>());
	pipeline_phong->add_shader(GL_VERTEX_SHADER, *shaders.find(ShaderId::InterfaceVertex));
	pipeline_phong->add_shader(GL_FRAGMENT_SHADER, *shaders.find(ShaderId::PhongFrag));
	pipelines.emplace("phong", pipeline_phong);

	auto pipeline_geometric_normals = std::static_pointer_cast<PipelineBase>(std::make_shared<PipelineGeometry>());
	pipeline_geometric_normals->add_shader(GL_VERTEX_SHADER, *shaders.find(ShaderId::InterfaceVertex));
	pipeline_geometric_normals->add_shader(GL_TESS_CONTROL_SHADER, *shaders.find(ShaderId::TessControl));
	pipeline_geometric_normals->add_shader(GL_TESS_EVALUATION_SHADER, *shaders.find(ShaderId::TessEval));
	pipeline_geometric_normals->add_shader(GL_GEOMETRY_SHADER, *shaders.find(ShaderId::GeometricNormalsGeom));
	pipeline_geometric_normals->add_shader(GL_FRAGMENT_SHADER, *shaders.find(ShaderId::SingleColorFrag));
	pipelines.emplace("vertex_normal", pipeline_geometric_normals);

	auto pipeline_displacement = std::static_pointer_cast<PipelineBase>(std::make_shared<PipelineDisplacement>());
	pipeline_displacement->add_shader(GL_VERTEX_SHADER, *shaders.find(ShaderId::InterfaceVertex));
	pipeline_displacement->add_shader(GL_TESS_CONTROL_SHADER, *shaders.find(ShaderId::TessControl));
	pipeline_displacement->add_shader(GL_TESS_EVALUATION_SHADER, *shaders.find(ShaderId::TessEval));
	pipeline_displacement->add_shader(GL_FRAGMENT_SHADER, *shaders.find(ShaderId::PhongFrag));
	pipelines.emplace("displacement", pipeline_displacement);

	auto pipeline_compute = std::static_pointer_cast<PipelineBase>(std::make_shared<PipelineCompute>());
	pipeline_compute->add_shader(GL_COMPUTE_SHADER, *shaders.find(ShaderId::AtmosphereScatteringLUT));
	pipelines.emplace("atmosphere scattering", pipeline_compute);

	auto pipeline_atmosphere = std::static_pointer_cast<PipelineBase>(std::make_shared<PipelineAtmosphere>());
	pipeline_atmosphere->add_shader(GL_VERTEX_SHADER, *shaders.find(ShaderId::ScreenQuadVertex));
	pipeline_atmosphere->add_shader(GL_FRAGMENT_SHADER, *shaders.find(ShaderId::AtmosphereRendering));
	pipelines.emplace("atmosphere", pipeline_atmosphere);

	for (const auto& [name, pipeline] : pipelines)
		pipeline->compile();

	return pipelines;
}

int main()
{
	Application app;
	app.launch();
	return 0;
}
