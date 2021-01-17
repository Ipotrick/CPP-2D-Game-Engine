#include <iostream>
#include <cassert>

#include "Window.hpp"

Window::~Window()
{
	close();
}

bool Window::open(std::string name, uint32_t width, uint32_t height)
{
	std::unique_lock l(mut);
	assert(!glfwWindow);		// can not open window moew than once without closing
	this->name = name;
	this->height = height;
	this->width = width;

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
	assert(!bRenderContextLocked);
	glfwDestroyWindow(glfwWindow);
	glfwWindow = nullptr;
}

void Window::update()
{
	std::unique_lock l(mut);
	// glfwPollEvents();
	if (bSetSize) {
		glfwSetWindowSize(glfwWindow, width, height);
		bSetSize = false;
	}
	else {
		glfwGetWindowSize(glfwWindow, (int*)&width, (int*)&height);
	}
}

bool Window::staticInit()
{
	if (!bGLEWInit) {
		if (!glfwInit()) {
			std::cerr << "ERROR: failed to initialize GLEW!" << std::endl;
			exit(-1);
		}
	}
	return true;
}