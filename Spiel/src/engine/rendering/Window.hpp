#pragma once

#include <string>
#include <mutex>
#include <cassert>

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "../types/ShortNames.hpp"

class Window {
public:
	~Window();

	bool operator=(const Window& window) const = delete;

	bool open(std::string name = "Default", u32 width = 1, u32 height = 1);

	void close();

	bool isOpen() const {
		std::unique_lock l(mut);
		return static_cast<bool>(glfwWindow);
	}

	void update();
	
	std::pair<u32, u32> getSize() const
	{
		std::unique_lock l(mut);
		return { width, height };
	}

	u32 getWidth() const {
		std::unique_lock l(mut);
		return width;
	}

	u32 getHeight() const {
		std::unique_lock l(mut);
		return height;
	}

	void setSize(u32 width, u32 height)
	{
		std::unique_lock l(mut);
		this->bSetSize = true;
		this->width = width;
		this->height = height;
	}

	void setWidth(u32 width)
	{
		std::unique_lock l(mut);
		this->bSetSize = true;
		this->width = width;
	}

	void setHeight(u32 height)
	{
		std::unique_lock l(mut);
		this->bSetSize = true;
		this->height = height;
	}

	std::string const& getName() const
	{
		std::unique_lock l(mut);
		return name;
	}

	void setName(std::string name)
	{
		std::unique_lock l(mut);
		this->name = std::move(name);
		glfwSetWindowTitle(glfwWindow, name.c_str());
	}

	void takeRenderingContext()
	{
		std::unique_lock lock(mut);
		assert(!bRenderContextLocked);
		glfwMakeContextCurrent(glfwWindow);
		if (glewInit() != GLEW_OK) {
			glfwTerminate();
		}
		bRenderContextLocked = true;
	}

	void releaseRenderingContext()
	{
		std::unique_lock l(mut);
		assert(bRenderContextLocked);
		glfwMakeContextCurrent(nullptr);
		bRenderContextLocked = false;
	}
	
	bool isRenderContextLocked() const
	{
		std::unique_lock l(mut);
		return bRenderContextLocked; // i don't think this is threadsave to use to lock the render context:/
	}

	bool isFocused() const
	{
		std::unique_lock l(mut);
		return glfwGetWindowAttrib(glfwWindow, GLFW_FOCUSED);
	}

	/**
	 * \return true when the user clicked the close icon on the window.
	 */
	bool shouldClose() const
	{
		std::unique_lock l(mut);
		return glfwWindowShouldClose(glfwWindow);
	}

	/**
	 * swapps rendering screen buffer of current opengl context render target 0 and the window.
	 */
	void swapBuffers()
	{
		glfwSwapBuffers(glfwWindow);
	}

	GLFWwindow* getNativeHandle() { return glfwWindow; }

private:
	mutable std::mutex mut{};
	friend class InputManager;

	bool bRenderContextLocked{ false };
	bool bSetSize{ false };
	u32 height{ 100 };
	u32 width{ 100 };
	std::string name;

	GLFWwindow* glfwWindow{ nullptr };

	static bool staticInit();
	inline static bool bGLEWInit{ staticInit() };
};