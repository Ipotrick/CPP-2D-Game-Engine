#pragma once

#include <string>
#include <mutex>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

class Window {
public:
	Window(std::string name_ = "Default", uint32_t width_ = 1, uint32_t height_ = 1):
		mut{},
		glfwWindow{nullptr},
		name{ name_ },
		height{ height_ },
		width{ width_ }
	{
		initialize();
	}
private:

	int initialize() {
		if (!glfwInit()) {
			return -1;
		}
		else {
			glfwWindowHint(GLFW_SAMPLES, 8);
			glfwWindow = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
			if (!glfwWindow) {
				glfwTerminate();
				return -2;
			}
		}
		return 0;
	}

public:
	std::mutex mut;
	GLFWwindow* glfwWindow;
	uint32_t height;
	uint32_t width;
	std::string name;
};