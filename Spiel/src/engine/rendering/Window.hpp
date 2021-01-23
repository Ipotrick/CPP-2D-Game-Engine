#pragma once

#include <string>
#include <mutex>
#include <cassert>
#include <array>
#include <vector>

#include "../types/ShortNames.hpp"
#include "../io/Input.hpp"
#include "../math/Vec2.hpp"

void keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods);

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
	Vec2 getSizeVec() const
	{
		std::unique_lock l(mut);
		return Vec2{ cast<f32>(width), cast<f32>(height) };
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

	bool keyPressed(Key key) const;
	bool keyJustPressed(Key key) const;
	bool keyReleased(Key key) const { return !keyJustPressed(key); }
	bool keyJustReleased(Key key) const;
	void consumeKeyEvent(Key key);
	std::vector<KeyEvent> getKeyEventsInOrder() const;

	bool buttonPressed(MouseButton button) const;
	bool buttonJustPressed(MouseButton button) const;
	bool buttonJustReleased(MouseButton button) const;
	void consumeMouseButtonEvent(MouseButton button);

	/**
	 * \return cursor position relative to the window in window coordinates (-1 < x < 1, -1 < y < 1, x to right, y to top);
	 */
	Vec2 getCursorPos() const;
	/**
	 * \return previous (the state before the last update) cursor position relative to the window in window coordinates (-1 < x < 1, -1 < y < 1, x to right, y to top);
	 */
	Vec2 getPrevCursorPos() const;

private:
	friend void keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods);

	mutable std::mutex mut{};
	friend class InputManager;

	bool bRenderContextLocked{ false };
	bool bSetSize{ false };
	u32 height{ 100 };
	u32 width{ 100 };
	std::string name;

	GLFWwindow* glfwWindow{ nullptr };

	// Input:
	std::vector<KeyEvent> keyEventsInOrder;
	std::array<bool, MAX_KEY_INDEX - MIN_KEY_INDEX + 1> previousKeyStates	= { false };
	std::array<bool, MAX_KEY_INDEX - MIN_KEY_INDEX + 1> keyStates			= { false };
	std::array<bool, MAX_KEY_INDEX - MIN_KEY_INDEX + 1> keyHide				= { false };
	std::array<bool, 8> previousMouseButtonStates							= { false };
	std::array<bool, 8> mouseButtonStates									= { false };
	std::array<bool, 8> mouseButtonHide										= { false };

	Vec2 cursorPosition{ 0.0f, 0.0f };
	Vec2 previousCursorPosition{ 0.0f, 0.0f };

};