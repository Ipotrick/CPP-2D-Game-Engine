#pragma once

#include <string>
#include <mutex>
#include <cassert>
#include <array>
#include <vector>

#include "../types/ShortNames.hpp"
#include "../io/Input.hpp"
#include "../math/Vec2.hpp"


class Window {
public:
	~Window();

	bool operator=(const Window& window) const = delete;
	bool operator=(Window&& window) const = delete;

	bool open(std::string name = "Default Window Name", u32 width = 1, u32 height = 1);

	void close();

	bool isOpen() const;

	void update(float deltaTime);
	
	std::pair<u32, u32> getSize() const;

	Vec2 getSizeVec() const;

	u32 getWidth() const;

	u32 getHeight() const;

	void setSize(u32 width, u32 height);

	void setWidth(u32 width);

	void setHeight(u32 height);

	std::string const& getName() const;

	void setName(std::string name);

	void takeRenderingContext();

	void returnRenderingContext();

	bool isFocused() const;

	/**
	 * \return true when the user clicked the close icon on the window.
	 */
	bool shouldClose() const;

	/**
	 * swapps rendering screen buffer of current opengl context render target 0 and the window.
	 */
	void swapBuffers();

	GLFWwindow* getNativeHandle();

	bool keyPressed(Key key) const;
	bool keyJustPressed(Key key) const;
	bool keyReleased(Key key) const { return !keyJustPressed(key); }
	bool keyJustReleased(Key key) const;
	bool keyRepeated(Key key) const;
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
	static void keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods);

	bool bRenderContextLocked{ false };
	mutable std::mutex mut{};
	bool bSetSize{ false };
	u32 height{ 100 };
	u32 width{ 100 };
	std::string name;

	GLFWwindow* glfwWindow{ nullptr };

	// Input:
	std::vector<KeyEvent> keyEventsInOrder;
	std::array<bool, MAX_KEY_INDEX - MIN_KEY_INDEX + 1> previousKeyStates	= { false };
	std::array<bool, MAX_KEY_INDEX - MIN_KEY_INDEX + 1> keysPressed			= { false };
	std::array<bool, MAX_KEY_INDEX - MIN_KEY_INDEX + 1> keysRepeated		= { false };
	std::array<bool, MAX_KEY_INDEX - MIN_KEY_INDEX + 1> keysHidden			= { false };
	inline static const float IN_BETWEEN_REPEAT_DELAY{ 0.04f };	// 40ms		delay between repeat events
	inline static const float INITIAL_REPEAT_DELAY{ 0.3f };		// 300ms	initial time waited until repeat events are created
	std::array<float, MAX_KEY_INDEX - MIN_KEY_INDEX + 1> keyRepeatTimer		= { IN_BETWEEN_REPEAT_DELAY };
	std::array<bool, 8> previousMouseButtonStates							= { false };
	std::array<bool, 8> mouseButtonStates									= { false };
	std::array<bool, 8> mouseButtonHide										= { false };

	Vec2 cursorPosition{ 0.0f, 0.0f };
	Vec2 previousCursorPosition{ 0.0f, 0.0f };

};