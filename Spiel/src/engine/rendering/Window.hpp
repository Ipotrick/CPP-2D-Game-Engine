#pragma once

#include <string>
#include <mutex>

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "../types/ShortNames.hpp"

class Window {
public:

	~Window();

	bool open(std::string name = "Default", u32 width = 1, u32 height = 1);

	void close();

	bool isOpen() const { return static_cast<bool>(glfwWindow); }

	/**
	 * fetches real size of the window from os.
	 */
	void updateSize()
	{
		glfwGetWindowSize(glfwWindow, (int*)&width, (int*)&height);
	}
	
	std::pair<u32, u32> getSize() const
	{
		return { width, height };
	}

	u32 getWidth() const { return width; }

	u32 getHeight() const { return height; }

	void setSize(u32 width, u32 height)
	{
		this->width = width;
		this->height = height;
	}

	void setWidth(u32 width)
	{
		this->width = width;
	}

	void setHeight(u32 height)
	{
		this->height = height;
	}

	std::string const& getName() const
	{
		return name;
	}

	void setName(std::string name)
	{
		this->name = std::move(name);
		glfwSetWindowTitle(glfwWindow, name.c_str());
	}

	/**
	 * makes the rendering context current on the calling thread.
	 * should be guarded by a lock of the renderingContextMut.
	 */
	void makeContextCurrent()
	{
		glfwMakeContextCurrent(glfwWindow);							// when we have two renderers we have to make sure that we actually have the context of the window
		if (glewInit() != GLEW_OK) {
			glfwTerminate();
		}
	}

	bool isFocused() const
	{
		return glfwGetWindowAttrib(glfwWindow, GLFW_FOCUSED);
	}

	/**
	 * \return true when the user clicked the close icon on the window.
	 */
	bool shouldClose() const
	{
		return glfwWindowShouldClose(glfwWindow);
	}

	/**
	 * swapps rendering screen buffer of current opengl context render target 0 and the window.
	 */
	void swapBuffers()
	{
		glfwSwapBuffers(glfwWindow);
	}

	std::mutex mut{};
	std::mutex renderingContextMut{};
private:
	friend class InputManager;

	u32 height{ 100 };
	u32 width{ 100 };
	std::string name;

	GLFWwindow* glfwWindow{ nullptr };

	static bool staticInit();
	inline static bool bGLEWInit{ staticInit() };
};