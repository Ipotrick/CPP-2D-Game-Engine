#pragma once

#include <mutex>
#include <condition_variable>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "Camera.h"
#include "Drawable.h"
#include "Window.h"
#include "Entity.h"
#include "World.h"
#include "Timing.h"
#include "glmath.h"

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

class RenderBuffer {
public:

	void writeBuffer(std::vector<Drawable> drawables_, Camera camera_) {
		drawables.clear();
		drawables.reserve(drawables_.size());
		for (auto& el : drawables_) {
			drawables.emplace_back(el);
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
		mut{  },
		cond{  }
	{}
	/* DO NOT ACCESS THESE, WHILE RENDER THREAD IS RUNNING, FROM OTHER THREAD */
	RenderBuffer renderBufferB;
	std::chrono::microseconds new_renderTime;
	std::chrono::microseconds new_renderSyncTime;
	bool run;
	/* ONLY USE THERE WITH A MUTEX LOCK */
	std::mutex mut;
	bool ready;
	std::condition_variable cond;
};

class Renderer
{
public:
	Renderer(std::shared_ptr<RendererSharedData> sharedData_, std::shared_ptr<Window> window_) :
		sharedData{ sharedData_ },
		window{window_}
	{
	}

	void initiate();
	void operator()();
	void end();
	int initGLWindow();


	std::string readShader(std::string path_);
	
private:
	std::shared_ptr<RendererSharedData> sharedData;
	GLFWwindow* glWindow;
	std::shared_ptr<Window> window;

	unsigned int shader;
	const std::string vertexShaderPath = "shader/Basic.vert";
	const std::string fragmentShaderPath = "shader/Basic.frag";
};

