#pragma once

#include <string>
#include <mutex>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

class Window {
public:
	Window();

	~Window();

	[[nodiscard]] bool open(std::string name_ = "Default", uint32_t width_ = 1, uint32_t height_ = 1);

	void close();

	std::mutex mut{};
	GLFWwindow* glfwWindow{ nullptr };
	uint32_t height{ 100 };
	uint32_t width{ 100 };
	std::string name{ "WindowName" };
private:
	inline static bool bGLEWInit{ false };
};