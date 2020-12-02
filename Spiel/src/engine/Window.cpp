#include <iostream>

#include "Window.hpp"

Window::Window()
{
	if (!bGLEWInit) {
		if (!glfwInit()) {
			std::cerr << "ERROR: failed to initialize GLEW!" << std::endl;
			exit(-1);
		}
	}
}

Window::~Window()
{
	close();
}

bool Window::open(std::string name_, uint32_t width_, uint32_t height_)
{
	std::unique_lock l(mut);
	name = name_;
	height = height_;
	width = width_;

	glfwWindowHint(GLFW_SAMPLES, 8);
	glfwWindow = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
	if (!glfwWindow) {
		glfwTerminate();
		return false;
	}
	return true;
}

void Window::close()
{
	std::unique_lock l(mut);
	if (glfwWindow) {
		glfwDestroyWindow(glfwWindow);
		glfwWindow = nullptr;
	}
}
