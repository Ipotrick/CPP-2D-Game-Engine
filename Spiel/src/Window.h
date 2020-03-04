#pragma once

#include <string>
#include <mutex>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

class Window {
public:
	Window(std::string name_ = "Default", uint32_t width_ = 1, uint32_t height_ = 1):
		mut{},
		name{ name_ },
		height{ height_ },
		width{ width_ }
	{
		
	}

public:
	std::mutex mut;
	uint32_t height;
	uint32_t width;
	std::string name;
};