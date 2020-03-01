#pragma once

#include <mutex>
#include <condition_variable>
#include <vector>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "Camera.h"
#include "Drawable.h"
#include "Window.h"
#include "Entity.h"
#include "World.h"
#include "Timing.h"

static unsigned int compileShader(unsigned int type_, const std::string source_) {
	unsigned id = glCreateShader(type_);
	char const* src = source_.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*)_malloca(length * sizeof(char));
		glGetShaderInfoLog(id, length, &length, message);
		std::cerr << "error: failed to compile shader: " << message << std::endl;
		glDeleteShader(id);
		return 0;
	}

	return id;
}

static unsigned createShader(const std::string& vertexShader_, const std::string& fragmentShader_) {
	unsigned program = glCreateProgram();
	unsigned vs = compileShader(GL_VERTEX_SHADER, vertexShader_);
	unsigned fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader_);

	assert(vs != 0);
	assert(fs != 0);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}

static std::string vertexShader =
"#version 410 core\n"
"\n"
"layout(location = 0) in vec4 position;\n"
"\n"
"void main() \n"
"{\n"
"	gl_Position = position;\n"
"}\n";

static std::string fragmentShader =
"#version 410 core\n"
"\n"
"layout(location = 0) out vec4 color;\n"
"\n"
"void main() \n"
"{\n"
"color = vec4(0.0, 1.0, 1.0, 0.5);\n"
"}\n";


class RenderBuffer {
public:
	RenderBuffer() {

	}

	void writeBuffer(std::vector<Drawable> drawables_, Camera camera_) {
		drawables.clear();
		drawables.reserve(drawables_.size());
		int i = 0;
		for (auto& el : drawables_) {
			drawables.emplace_back(drawables_.at(i++));
		}
		camera = camera_;
	}
public:
	std::vector<Drawable> drawables;
	Camera camera;
};

struct RendererSharedData {
	RendererSharedData(): 
		run{ true },
		ready{ false },
		new_renderTime{ 0 },
		renderBufferB{  },
		mut{  },
		cond{  }
	{}
	/* DO NOT ACCESS THESE WHILE RENDER THREAD IS RUNNING FROM OTHER THREAD */
	std::chrono::microseconds new_renderTime;
	RenderBuffer renderBufferB;
	bool run;
	/* ONLY USE THERE WITH A MUTEX LOCK */
	std::mutex mut;
	bool ready;
	std::condition_variable cond;
};

class Renderer
{
public:
	Renderer(std::shared_ptr<Window> window_, std::shared_ptr<RendererSharedData> sharedData_) :
		window{window_},
		sharedData{ sharedData_ }
	{
	}

	void initiate();

	void operator()();
	
private:
	std::shared_ptr<Window> window;
	std::shared_ptr<RendererSharedData> sharedData;
	unsigned int shader;
};

