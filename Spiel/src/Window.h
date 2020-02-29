#pragma once

class Window {
public:
	Window(std::string name_, uint32_t width_, uint32_t height_):
		name{ name_ },
		height{ height_ },
		width{ width_ }
	{
	}
private:
	uint32_t height;
	uint32_t width;
	std::string name;
};