#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>

#include <iostream>
#include <memory>
#include <string>
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

	void init_from_source(const std::string& vertex, const std::string& frag) {
		m_id = GLptr(
			[](GLuint* ptr) { *ptr = glCreateProgram(); },
			[](const GLuint* ptr) { glDeleteProgram(*ptr); }
		);

		auto create_shader = [](const GLenum type) {
			return GLptr([type](GLuint* ptr) { *ptr = glCreateShader(type); },
				[](const GLuint* ptr) { glDeleteShader(*ptr); });
		};

		GLptr vertex_gl = create_shader(GL_VERTEX_SHADER);
		GLptr fragment_gl = create_shader(GL_FRAGMENT_SHADER);

		struct ShaderCode {
			GLuint id;
			const char* code;
		};
		std::vector<ShaderCode> shaders_code = { { vertex_gl, vertex.c_str()} , { fragment_gl, frag.c_str() } };

		for (const auto& shader : shaders_code) {
			glShaderSource(shader.id, 1, &shader.code, NULL);
			glCompileShader(shader.id);
			{
				int infoLogLength;
				GLint compileStatus = GL_FALSE;
				glGetShaderiv(shader.id, GL_COMPILE_STATUS, &compileStatus);
				glGetShaderiv(shader.id, GL_INFO_LOG_LENGTH, &infoLogLength);
				if (infoLogLength > 1) {
					std::vector<char> errorMessage(static_cast<size_t>(infoLogLength) + 1);
					glGetShaderInfoLog(shader.id, infoLogLength, NULL, errorMessage.data());
					std::cout << "shader " << (compileStatus ? "warning" : "error") << " : " << std::string(errorMessage.data()) << std::endl;
					std::cout << std::endl << std::endl << shader.code << std::endl;
				}
			}
			glAttachShader(m_id, shader.id);
		}

		glLinkProgram(m_id);
		{
			int infoLogLength;
			GLint linkStatus = GL_FALSE;
			glGetProgramiv(m_id, GL_LINK_STATUS, &linkStatus);
			glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &infoLogLength);
			if (infoLogLength > 1) {
				std::vector<char> errorMessage(static_cast<size_t>(infoLogLength) + 1);
				glGetProgramInfoLog(m_id, infoLogLength, NULL, errorMessage.data());
				std::cout << "shader program " << (linkStatus ? "warning" : "error") << " : " << std::string(errorMessage.data()) << std::endl;
			}
		}

		for (const auto& shader : shaders_code) {
			glDetachShader(m_id, shader.id);
		}
	}

	void use() {
		glUseProgram(m_id);
	}

	template<typename F, typename ...Args>
	void set_uniform(const char* name, F&& f, Args&& ...args) {
		const GLint location = glGetUniformLocation(m_id, name);
		f(location, std::forward<Args>(args)...);
	}

	GLptr m_id;
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

struct GLvao {

	GLvao() 
	{
		m_id = GLptr(
			[](GLuint* ptr) { glGenVertexArrays(1, ptr); },
			[](const GLuint* ptr) { glDeleteVertexArrays(1, ptr); }
		);
	}

	void bind()
	{
		glBindVertexArray(m_id);
	}

	GLptr m_id;
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