#pragma once

#include <string>
#include <mutex>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

class Window {
public:
	Window(std::string name_, uint32_t width_, uint32_t height_):
		mut{  },
		name{ name_ },
		height{ height_ },
		width{ width_ }
	{
		
	}

	int init() {
		if (!glfwInit()) {
			return -1;
		}
		else {
			glWindow = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
			if (!glWindow) {
				glfwTerminate();
				return -2;
			}
			else {
				glfwMakeContextCurrent(glWindow);

				if (glewInit() != GLEW_OK) {
					glfwTerminate();
					return -3;
				}
			}
		}
		return 0;
	}

	void setTitle(char const* name_) {
		name = name_;
	}
	void setHeight(uint32_t height_) {
		height = height_;
	}
	void setWidth(uint32_t width_) {
		width = width_;
	}

	std::string getTitle() {
		return name;
	}
	uint32_t getHeight() {
		return height;
	}
	uint32_t getWidth() {
		return width;
	}

public:
	std::mutex mut;
	GLFWwindow* glWindow;
private:
	uint32_t height;
	uint32_t width;
	std::string name;
};