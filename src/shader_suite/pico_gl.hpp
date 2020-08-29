#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>

#include <any>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define GL_ENUM_STR(name) { name, #name }
inline void gl_check()
{
	static const std::map<GLenum, std::string> errors = {
		GL_ENUM_STR(GL_INVALID_ENUM),
		GL_ENUM_STR(GL_INVALID_VALUE),
		GL_ENUM_STR(GL_INVALID_OPERATION),
		GL_ENUM_STR(GL_STACK_OVERFLOW),
		GL_ENUM_STR(GL_STACK_UNDERFLOW),
		GL_ENUM_STR(GL_OUT_OF_MEMORY),
		GL_ENUM_STR(GL_INVALID_FRAMEBUFFER_OPERATION)
	};

	const GLenum gl_err = glGetError();
	if (gl_err) {
		std::string err;
		auto it = errors.find(gl_err);
		if (it != errors.end()) {
			err = it->second;
		} else {
			err = "UNKNOWN_GL_ERROR";
		}
	}
}

inline void gl_framebuffer_check(GLenum target)
{
	static const std::map<GLenum, std::string> errors = {
		GL_ENUM_STR(GL_FRAMEBUFFER_UNDEFINED),
		GL_ENUM_STR(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT),
		GL_ENUM_STR(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT),
		GL_ENUM_STR(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER),
		GL_ENUM_STR(GL_FRAMEBUFFER_UNSUPPORTED),
		GL_ENUM_STR(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE),
		GL_ENUM_STR(GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS)
	};

	const GLenum status = glCheckFramebufferStatus(target);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		std::string err;
		auto it = errors.find(status);
		if (it != errors.end()) {
			err = it->second;
		} else {
			err = "UNKNOWN_FRAMEBUFFER_ERROR";
		}
		std::cout << err << std::endl;
	}
}
#undef GL_ENUM_STR

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

		GLptr shader = GLptr(
			[type](GLuint* ptr) { *ptr = glCreateShader(type); },
			[](const GLuint* ptr) { glDeleteShader(*ptr); }
		);

		const char* code = code_str.c_str();
		glShaderSource(shader, 1, &code, NULL);
		glCompileShader(shader);
		{
			GLint compile_status = GL_FALSE, log_length;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
			if (log_length > 1) {
				std::vector<char> error_message(static_cast<size_t>(log_length) + 1);
				glGetShaderInfoLog(shader, log_length, NULL, error_message.data());
				std::cout << "shader " << (compile_status ? "warning" : "error") << " : " << error_message.data() << std::endl;
				std::cout << std::endl << std::endl << code_str << std::endl;
			}
		}
		glAttachShader(m_id, shader);

		m_shaders[type] = shader;
		m_link_status = GL_FALSE;
	}

	void use()
	{
		if (!m_link_status && !m_shaders.empty()) {
			glLinkProgram(m_id);
			{
				int log_length;
				glGetProgramiv(m_id, GL_LINK_STATUS, &m_link_status);
				glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &log_length);
				if (log_length > 1) {
					std::vector<char> error_message(static_cast<size_t>(log_length) + 1);
					glGetProgramInfoLog(m_id, log_length, NULL, error_message.data());
					std::cout << "shader program " << (m_link_status ? "warning" : "error") << " : " << error_message.data() << std::endl;
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

struct GLformat
{
	GLformat() = default;
	GLformat(const GLenum internal, const GLenum format, const GLenum type)
		: m_internal(internal), m_format(format), m_type(type) { }

	GLenum m_internal = GL_RGBA8, m_format = GL_RGBA, m_type = GL_UNSIGNED_BYTE;
};

struct GLTexture
{
	GLTexture() = default;
	GLTexture(const GLformat format, const GLsizei sample_count = 1)
		: m_format(format)
	{
		set_sample_count(sample_count);
	}

	bool has_multi_samples() const {
		return m_sample_count > 1;
	}

	void set_sample_count(const GLsizei count) {
		if (m_sample_count != count) {
			m_sample_count = count;
		}

		if (has_multi_samples()) {
			m_target = GL_TEXTURE_2D_MULTISAMPLE;
		} else {
			m_target = GL_TEXTURE_2D;
		}
	}

	void load(const std::string& filepath) {
		int x, y, comp;
		unsigned char* data = stbi_load(filepath.c_str(), &x, &y, &comp, 0);
		if (!data) {
			std::cout << "cant load " << filepath << std::endl;
		}
		set_sample_count(1);
		init_gl(x, y, {}, data);
		stbi_image_free(data);
	}

	void bind() {
		glBindTexture(m_target, m_gl);
	}

	void resize(const int w, const int h)
	{
		if (w == m_w && h == m_h) {
			return;
		}
		init_gl(w, h, m_format);
	}

	void init_gl(const int w, const int h, const GLformat gl_format = {}, const void* data = nullptr) {
		m_gl = GLptr([](GLuint* ptr) { glGenTextures(1, ptr); },
			[](const GLuint* ptr) { glDeleteTextures(1, ptr); });

		bind();

		m_format = gl_format;
		m_w = w;
		m_h = h;

		gl_check();

		if (has_multi_samples()) {
			glTexStorage2DMultisample(m_target, m_sample_count, m_format.m_internal, m_w, m_h, GL_TRUE);
			gl_check();
		} else {
			GLsizei lod_count = static_cast<GLsizei>(std::ceil(std::log(std::max(w, h)))) + 1;
			glTexStorage2D(m_target, lod_count, gl_format.m_internal, w, h);

			if (data) {
				glTexSubImage2D(m_target, 0, 0, 0, w, h, gl_format.m_format, gl_format.m_type, data);
				generate_mipmap();
			}
		}

		glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(m_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(m_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	void generate_mipmap() {
		bind();
		glGenerateMipmap(m_target);
	}

	void set_swizzle_mask(const GLint r = GL_RED, const GLint g = GL_GREEN, const GLint b = GL_BLUE, const GLint a = GL_ALPHA) {
		std::array<GLint, 4> swizzle_mask = { r, g, b, a };
		bind();
		glTexParameteriv(m_target, GL_TEXTURE_SWIZZLE_RGBA, swizzle_mask.data());
	}

	void bind_slot(const GLuint slot) {
		glActiveTexture(slot);
		bind();
	}

	void bind_as_image_texture()
	{
		bind();
		glBindImageTexture(0, m_gl, 0, GL_FALSE, 0, GL_WRITE_ONLY, m_format.m_internal);
	}

	GLptr m_gl;
	GLformat m_format;
	GLenum m_target = GL_TEXTURE_2D;
	GLsizei m_sample_count = 1;
	int m_w = 0, m_h = 0;
};

struct GLFramebuffer
{
	GLFramebuffer() = default;

	void init_gl()
	{
		m_gl = GLptr(
			[](GLuint* ptr) { glGenFramebuffers(1, ptr); },
			[](const GLuint* ptr) { glDeleteFramebuffers(1, ptr); }
		);
	}

	void init_depth()
	{
		assert(m_gl);

		m_depth_id = GLptr(
			[](GLuint* ptr) { glGenRenderbuffers(1, ptr); },
			[](const GLuint* ptr) { glDeleteRenderbuffers(1, ptr); }
		);
		glBindRenderbuffer(GL_RENDERBUFFER, m_depth_id);
		if (m_sample_count > 1) {
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_sample_count, GL_DEPTH_COMPONENT32, m_w, m_h);
		} else {
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, m_w, m_h);
		}
		bind();
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth_id);

	}

	void clear(const float r = 0.0f, const float g = 0.0f, const float b = 0.0f, const float a = 1.0f)
	{
		bind();
		glClearColor(r, g, b, a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void bind(const GLenum target = GL_FRAMEBUFFER) const
	{
		glBindFramebuffer(target, m_gl);
	}

	void bind_read(const GLenum attachment) const {
		bind(GL_READ_FRAMEBUFFER);
		glReadBuffer(attachment);
	}

	void bind_draw() {
		bind(GL_DRAW_FRAMEBUFFER);
		std::vector<GLenum> attachments(m_attachments.size());
		for (std::size_t i = 0; i < m_attachments.size(); ++i) {
			attachments[i] = GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i);
		}
		glDrawBuffers(static_cast<GLsizei>(m_attachments.size()), attachments.data());
	}

	void bind_draw(const GLenum attachment) {
		bind(GL_DRAW_FRAMEBUFFER);
		glDrawBuffers(1, &attachment);
	}

	void blit_from(const GLFramebuffer& from, const GLenum filter = GL_NEAREST, const GLenum attachment_from = GL_COLOR_ATTACHMENT0, const GLenum attachment_to = GL_COLOR_ATTACHMENT0) {
		bind_draw(attachment_to);
		from.bind_read(attachment_from);
		glBlitFramebuffer(0, 0, from.m_w, from.m_h, 0, 0, m_w, m_h, GL_COLOR_BUFFER_BIT, filter);
	}

	void blit_inversed_from(const GLFramebuffer& from, const GLenum filter = GL_NEAREST, const GLenum attachment_from = GL_COLOR_ATTACHMENT0, const GLenum attachment_to = GL_COLOR_ATTACHMENT0) {
		bind_draw(attachment_to);
		from.bind_read(attachment_from);
		glBlitFramebuffer(0, 0, from.m_w, from.m_h, 0, m_h, m_w, 0, GL_COLOR_BUFFER_BIT, filter);
	}

	void resize(const int w, const int h, const GLsizei sample_count = 1)
	{
		if (m_w == w && m_h == h) {
			return;
		}
		m_w = w;
		m_h = h;

		init_gl();
		init_depth();

		std::vector<GLTexture> tmp_attachments = m_attachments;
		m_attachments.clear();
		for (const GLTexture& attachment : tmp_attachments) {
			create_attachment(attachment.m_format);
		}

		gl_framebuffer_check(GL_FRAMEBUFFER);
	}

	void add_attachment(const GLTexture& texture, const GLenum target = GL_FRAMEBUFFER)
	{
		assert(texture.m_sample_count == m_sample_count);
		if (m_attachments.empty() && (m_w == 0 || m_h == 0)) {
			m_w = texture.m_w;
			m_h = texture.m_h;
		}
		bind(target);
		glFramebufferTexture2D(target, GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(m_attachments.size()), texture.m_target, texture.m_gl, 0);
		m_attachments.push_back(texture);
	}

	template<typename ... Texture>
	void add_attachments(const Texture& ... texture)
	{
		((add_attachment(texture)), ...);
	}

	void create_attachment(const GLformat format)
	{
		GLTexture attachement;
		attachement.set_sample_count(m_sample_count);
		attachement.init_gl(m_w, m_h, format);
		add_attachment(attachement);
	}

	template<typename ...Formats>
	void create_attachments(const Formats ... formats) {
		((create_attachment(formats)), ...);
	}

	static GLFramebuffer& get_default(const int w, const int h)
	{
		static GLFramebuffer default_framebuffer;
		default_framebuffer.m_gl.m_id = std::make_shared<GLuint>(0u);
		default_framebuffer.m_w = w;
		default_framebuffer.m_h = h;
		return default_framebuffer;
	}

	std::vector<GLTexture> m_attachments;
	GLptr m_gl, m_depth_id;
	GLsizei m_sample_count = 1;
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
			m_vertices_count = static_cast<GLsizei>(attribute.size());
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

	void draw(const GLenum mode = GL_TRIANGLES)
	{
		assert(m_vao);

		glBindVertexArray(m_vao);
		if (m_indices_buffer) {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indices_buffer);
			glDrawElements(mode, m_indices_count, GL_UNSIGNED_INT, 0);
		} else {
			glDrawArrays(mode, 0, m_indices_count);
		}
		glBindVertexArray(0);
	}

	GLptr m_vao, m_indices_buffer, m_vertex_buffer;
	GLsizei m_indices_count = 0, m_vertices_count = 0;
};

template<typename BeforeClearFun, typename LoopFun>
void create_context_and_run(BeforeClearFun&& before_clear_fun, LoopFun&& loop_fun)
{
	if (!glfwInit()) {
		std::cout << "glfwInit Initialization failed " << std::endl;
		return;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	if (!mode) {
		return;
	}
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

	const int w = mode->width, h = mode->height;
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
	std::cout << "glad version : " << version << std::endl;
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
		before_clear_fun();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

		loop_fun(window.get());

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