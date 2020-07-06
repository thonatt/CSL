#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>

#include <any>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

class GLptr {

public:
	GLptr() = default;

	template<typename Creator, typename Destructor>
	GLptr(Creator&& creator, Destructor&& destructor) {
		m_id = std::shared_ptr<GLuint>(new GLuint(), std::forward<Destructor>(destructor));
		creator(m_id.get());
	}

	operator GLuint() const {
		return m_id ? *m_id : 0;
	}

protected:
	std::shared_ptr<GLuint> m_id;
};

struct GLProgram {

	GLProgram() = default;

	void set_shader_glsl(GLenum type, const std::string& code_str)
	{
		if (!m_id) {
			m_id = GLptr(
				[](GLuint* ptr) { *ptr = glCreateProgram(); },
				[](const GLuint* ptr) { glDeleteProgram(*ptr); }
			);
		}

		GLptr shader = GLptr([type](GLuint* ptr) { *ptr = glCreateShader(type); },
			[](const GLuint* ptr) { glDeleteShader(*ptr); });

		const char* code = code_str.c_str();
		glShaderSource(shader, 1, &code, NULL);
		glCompileShader(shader);
		{
			int infoLogLength;
			GLint compileStatus = GL_FALSE;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
			if (infoLogLength > 1) {
				std::vector<char> errorMessage(static_cast<size_t>(infoLogLength) + 1);
				glGetShaderInfoLog(shader, infoLogLength, NULL, errorMessage.data());
				std::cout << "shader " << (compileStatus ? "warning" : "error") << " : " << std::string(errorMessage.data()) << std::endl;
				std::cout << std::endl << std::endl << code_str << std::endl;
			}
		}
		glAttachShader(m_id, shader);

		m_shaders[type] = shader;
	}

	void use() {
		if (!m_link_status && !m_shaders.empty()) {
			glLinkProgram(m_id);
			{
				int infoLogLength;
				glGetProgramiv(m_id, GL_LINK_STATUS, &m_link_status);
				glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &infoLogLength);
				if (infoLogLength > 1) {
					std::vector<char> errorMessage(static_cast<size_t>(infoLogLength) + 1);
					glGetProgramInfoLog(m_id, infoLogLength, NULL, errorMessage.data());
					std::cout << "shader program " << (m_link_status ? "warning" : "error") << " : " << std::string(errorMessage.data()) << std::endl;
				}
			}

			for (const auto& shader : m_shaders) {
				glDetachShader(m_id, shader.second);
			}

			m_shaders.clear();
		}

		if (m_link_status) {
			glUseProgram(m_id);
		}	
	}

	template<typename F, typename ...Args>
	void set_uniform(const char* name, F&& f, Args&& ...args) {
		const GLint location = glGetUniformLocation(m_id, name);
		f(location, std::forward<Args>(args)...);
	}

	GLptr m_id;
	std::unordered_map<GLenum, GLptr> m_shaders;
	GLint m_link_status = GL_FALSE;
};

struct GLTexture {

	GLTexture() = default;

	void load(const std::string& filepath) {
		int x, y, comp;
		unsigned char* data = stbi_load(filepath.c_str(), &x, &y, &comp, 0);
		if (!data) {
			std::cout << "cant load " << filepath << std::endl;
		}
		init_gl(x, y, data);
		stbi_image_free(data);
	}

	void bind() {
		glBindTexture(GL_TEXTURE_2D, m_gl);
	}

	void init_gl(const int w, const int h, const void* data = nullptr) {
		m_gl = GLptr([](GLuint* ptr) { glGenTextures(1, ptr); },
			[](const GLuint* ptr) { glDeleteTextures(1, ptr); });

		bind();
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, w, h);
		if (data) {
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	void bind_slot(const GLuint slot) {
		glActiveTexture(slot);
		bind();
	}

	GLptr m_gl;
};

struct GLFramebuffer {
	GLFramebuffer() = default;

	void clear()
	{
		bind();
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void bind(GLenum target = GL_FRAMEBUFFER)
	{
		glBindFramebuffer(target, m_id);
	}

	void resize(const int w, const int h)
	{
		if (m_w == w && m_h == h) {
			return;
		}

		m_w = w;
		m_h = h;

		m_id = GLptr(
			[](GLuint* ptr) { glGenFramebuffers(1, ptr); },
			[](const GLuint* ptr) { glDeleteFramebuffers(1, ptr); }
		);

		m_color.init_gl(w, h);

		m_depth_id = GLptr(
			[](GLuint* ptr) { glGenRenderbuffers(1, ptr); },
			[](const GLuint* ptr) { glDeleteRenderbuffers(1, ptr); }
		);

		glBindRenderbuffer(GL_RENDERBUFFER, m_depth_id);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, w, h);

		bind();
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth_id);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color.m_gl, 0);
	}

	GLTexture m_color;
	GLptr m_id, m_depth_id;
	int m_w = 0, m_h = 0;

};

struct GLmesh {
	GLmesh() = default;

	void init()
	{
		m_vao = GLptr(
			[](GLuint* ptr) { glGenVertexArrays(1, ptr); },
			[](const GLuint* ptr) { glDeleteVertexArrays(1, ptr); }
		);
	}

	void set_triangles(const std::vector<GLuint>& indices) {
		m_indices_buffer = GLptr(
			[](GLuint* ptr) { glGenBuffers(1, ptr); },
			[](const GLuint* ptr) { glDeleteBuffers(1, ptr); }
		);
		assert(m_vao);
		assert(!indices.empty());

		glBindVertexArray(m_vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indices_buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_STATIC_DRAW);
		m_indices_count = static_cast<GLsizei>(indices.size());
	}

	template<typename ...Ts>
	void set_attributes(const std::vector<Ts>& ... attributes) {
		set_attributes(std::make_index_sequence<sizeof...(Ts)>{}, attributes...);
	}

	template<std::size_t ...Is, typename ...Ts>
	void set_attributes(std::index_sequence<Is...> const&, const std::vector<Ts>& ... attributes) {
		assert(m_vao);

		auto get_size = [](const auto& attribute) {
			return attribute.size() * sizeof(attribute[0]);
		};

		const std::size_t vertex_data_size = (get_size(attributes) + ...);
		std::vector<char> vertex_data(vertex_data_size);

		std::size_t offset = 0;
		auto setup_attribute = [&](const GLuint index, const auto& attribute) {
			assert(get_size(attribute) > 0);

			std::memcpy(vertex_data.data() + offset, attribute.data(), get_size(attribute));
			glVertexAttribPointer(index, sizeof(attribute[0]) / sizeof(float), GL_FLOAT, GL_FALSE, 0, ((char*)0) + offset);
			glEnableVertexAttribArray(index);
			offset += get_size(attribute);
			m_vertice_count = static_cast<GLsizei>(attribute.size());
		};

		m_vertex_buffer = GLptr(
			[](GLuint* ptr) { glGenBuffers(1, ptr); },
			[](const GLuint* ptr) { glDeleteBuffers(1, ptr); }
		);

		glBindVertexArray(m_vao);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);

		(setup_attribute(Is, attributes), ...);

		glBufferData(GL_ARRAY_BUFFER, vertex_data.size(), vertex_data.data(), GL_STATIC_DRAW);
	}

	void draw()
	{
		assert(m_vao);

		glBindVertexArray(m_vao);
		if (m_indices_buffer) {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indices_buffer);
			glDrawElements(GL_TRIANGLES, m_indices_count, GL_UNSIGNED_INT, 0);
		} else {
			glDrawArrays(GL_TRIANGLES, 0, m_indices_count);
		}
		glBindVertexArray(0);
	}

	GLptr m_vao, m_indices_buffer, m_vertex_buffer;
	GLsizei m_indices_count = 0, m_vertice_count = 0;
};

template<typename F>
void create_context_and_run(F&& f)
{
	if (!glfwInit()) {
		std::cout << "glfwInit Initialization failed " << std::endl;
		return;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	if (!mode) {
		return;
	}
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

	const int w = 1600, h = 1000;
	auto window = std::shared_ptr<GLFWwindow>(glfwCreateWindow(w, h, "csl shader suite demo", NULL, NULL), glfwDestroyWindow);

	if (!window) {
		std::cout << " Window or OpenGL context creation failed " << std::endl;
	}

	glfwMakeContextCurrent(window.get());
	const int desired_fps = 60;
	const int interval = mode->refreshRate / desired_fps;
	glfwSwapInterval(interval);

	glfwSetMouseButtonCallback(window.get(), ImGui_ImplGlfw_MouseButtonCallback);
	glfwSetKeyCallback(window.get(), ImGui_ImplGlfw_KeyCallback);
	glfwSetCharCallback(window.get(), ImGui_ImplGlfw_CharCallback);
	glfwSetScrollCallback(window.get(), ImGui_ImplGlfw_ScrollCallback);

	const int version = gladLoadGL();
	std::cout << "OpenGL version " << GLVersion.major << "." << GLVersion.minor << std::endl;

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui::StyleColorsDark();

	//imgui scaling
	float xscale, yscale;
	glfwGetMonitorContentScale(glfwGetPrimaryMonitor(), &xscale, &yscale);

	const float scaling = std::max(xscale, yscale);
	ImGui::GetStyle().ScaleAllSizes(scaling);
	ImGui::GetIO().FontGlobalScale = scaling;

	ImGui_ImplGlfw_InitForOpenGL(window.get(), false);
	ImGui_ImplOpenGL3_Init("#version 410");

	while (!glfwWindowShouldClose(window.get())) {

		glClear((GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGuiIO& io = ImGui::GetIO();
		if (io.KeysDown[GLFW_KEY_ESCAPE]) {
			glfwSetWindowShouldClose(window.get(), GLFW_TRUE);
		}

		if (io.KeysDown[GLFW_KEY_LEFT_CONTROL] && io.MouseWheel != 0) {
			static float scale = scaling;
			scale *= std::pow(1.05f, io.MouseWheel);

			ImGuiStyle style;
			style.ScaleAllSizes(scale);
			ImGui::GetStyle() = style;
			ImGui::GetIO().FontGlobalScale = scale;
		}

		f();

		static bool show_imgui = false;
		if (io.KeysDownDuration[GLFW_KEY_H] == 0) {
			show_imgui = !show_imgui;
		}
		if (show_imgui) {
			ImGui::ShowDemoWindow();
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, w, h);
		glClearColor(0, 0, 0, 0);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window.get());
	}
}