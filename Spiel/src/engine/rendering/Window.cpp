#include "Window.hpp"

#include <iostream>
#include <cassert>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

void Window::keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods)
{
	u32 keyIndex = key - MIN_KEY_INDEX;
	Window* window = (Window*)glfwGetWindowUserPointer(win);
	if (action == GLFW_PRESS && !window->keysPressed[keyIndex]) {
		window->keyEventsInOrder.push_back(KeyEvent{ cast<Key>(key), KeyEvent::Type::JustPressed });
	}
	else if (action == GLFW_RELEASE && window->keysPressed[keyIndex]) {
		window->keyEventsInOrder.push_back(KeyEvent{ cast<Key>(key), KeyEvent::Type::JustReleased });
	}
}

Window::~Window()
{
	if (glfwWindow) {
		close();
	}
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

	glfwSetKeyCallback(glfwWindow, keyCallback);
	glfwSetWindowUserPointer(glfwWindow, this);

	return true;
}

void Window::close()
{
	std::unique_lock l(mut);
	assert(glfwWindow);
	glfwDestroyWindow(glfwWindow);
	glfwWindow = nullptr;
}

bool Window::isOpen() const
{
	std::unique_lock l(mut);
	return static_cast<bool>(glfwWindow);
}

void Window::update(float deltaTime)
{
	std::unique_lock l(mut);
	keyEventsInOrder.clear();
	glfwPollEvents();
	if (bSetSize) {
		glfwSetWindowSize(glfwWindow, width, height);
		bSetSize = false;
	}
	else {
		glfwGetWindowSize(glfwWindow, (int*)&width, (int*)&height);
	}
	for (u32 keyIndex = 0; keyIndex < (MAX_KEY_INDEX - MIN_KEY_INDEX + 1); keyIndex++) {
		previousKeyStates[keyIndex] = keysPressed[keyIndex];

		keysHidden[keyIndex] = false;

		keysPressed[keyIndex] = cast<bool>(glfwGetKey(glfwWindow, keyIndex + MIN_KEY_INDEX));

		keysRepeated[keyIndex] = false;
		if (keysPressed[keyIndex]) {
			keyEventsInOrder.push_back(KeyEvent{ cast<Key>(keyIndex + MIN_KEY_INDEX), KeyEvent::Type::Pressed });
			keyRepeatTimer[keyIndex] -= deltaTime;
			if (keyRepeatTimer[keyIndex] <= 0.0f) {
				while (keyRepeatTimer[keyIndex] <= 0.0f) {
					keyRepeatTimer[keyIndex] += IN_BETWEEN_REPEAT_DELAY;
				}
				keyEventsInOrder.push_back(KeyEvent{ cast<Key>(keyIndex + MIN_KEY_INDEX), KeyEvent::Type::Repeat });
				keysRepeated[keyIndex] = true;
			}
		}
		else {
			keyRepeatTimer[keyIndex] = INITIAL_REPEAT_DELAY;
		}
	}
	for (u32 buttonIndex = 0; buttonIndex < (MAX_MOUSE_BUTTON_INDEX + 1); buttonIndex++) {
		previousMouseButtonStates[buttonIndex] = mouseButtonStates[buttonIndex];
		mouseButtonStates[buttonIndex] = cast<bool>(glfwGetMouseButton(glfwWindow, buttonIndex + MIN_MOUSE_BUTTON_INDEX));
		mouseButtonHide[buttonIndex] = false;
	}
	previousCursorPosition = cursorPosition;
	double pixelSpaceX, pixelSpaceY;
	glfwGetCursorPos(glfwWindow, &pixelSpaceX, &pixelSpaceY);
	float windowSpaceX = cast<f32>((pixelSpaceX / width * 2.0f) - 1.0f);
	float windowSpaceY = cast<f32>(-((pixelSpaceY / height * 2.0f) - 1.0f));
	cursorPosition = { windowSpaceX, windowSpaceY };
}

std::pair<u32, u32> Window::getSize() const
{
	std::unique_lock l(mut);
	return { width, height };
}

Vec2 Window::getSizeVec() const
{
	std::unique_lock l(mut);
	return Vec2{ cast<f32>(width), cast<f32>(height) };
}

u32 Window::getWidth() const
{
	std::unique_lock l(mut);
	return width;
}

u32 Window::getHeight() const
{
	std::unique_lock l(mut);
	return height;
}

void Window::setSize(u32 width, u32 height)
{
	std::unique_lock l(mut);
	this->bSetSize = true;
	this->width = width;
	this->height = height;
}

void Window::setWidth(u32 width)
{
	std::unique_lock l(mut);
	this->bSetSize = true;
	this->width = width;
}

void Window::setHeight(u32 height)
{
	std::unique_lock l(mut);
	this->bSetSize = true;
	this->height = height;
}

std::string const& Window::getName() const
{
	std::unique_lock l(mut);
	return name;
}

void Window::setName(std::string name)
{
	std::unique_lock l(mut);
	this->name = std::move(name);
	glfwSetWindowTitle(glfwWindow, name.c_str());
}

void Window::takeRenderingContext()
{
	std::unique_lock lock(mut);
	assert(!this->bRenderContextLocked);
	glfwMakeContextCurrent(glfwWindow);
	if (glewInit() != GLEW_OK) {
		glfwTerminate();
	}
	this->bRenderContextLocked = true;
}

void Window::returnRenderingContext()
{
	assert(this->bRenderContextLocked);
	this->bRenderContextLocked = false;
}

bool Window::isFocused() const
{
	std::unique_lock l(mut);
	return glfwGetWindowAttrib(glfwWindow, GLFW_FOCUSED);
}

/**
* \return true when the user clicked the close icon on the window.
*/

bool Window::shouldClose() const
{
	std::unique_lock l(mut);
	return glfwWindowShouldClose(glfwWindow);
}


/**
* swapps rendering screen buffer of current opengl context render target 0 and the window.
*/

void Window::swapBuffers()
{
	std::unique_lock lock(mut);
	assert(glfwWindow);
	glfwSwapBuffers(glfwWindow);
}

GLFWwindow* Window::getNativeHandle()
{
	std::unique_lock lock(mut);
	return glfwWindow;
}

bool Window::keyPressed(Key key) const
{
	std::unique_lock lock(mut);
	return (!keysHidden[cast<u32>(key) - MIN_KEY_INDEX]) and keysPressed[cast<u32>(key) - MIN_KEY_INDEX];
}

bool Window::keyRepeated(Key key) const
{
	std::unique_lock lock(mut);
	return !keysHidden[cast<u32>(key) - MIN_KEY_INDEX] and
		keysRepeated[cast<u32>(key) - MIN_KEY_INDEX];
}

bool Window::keyJustPressed(Key key) const
{
	std::unique_lock lock(mut);
	return !keysHidden[cast<u32>(key) - MIN_KEY_INDEX] and 
			keysPressed[cast<u32>(key) - MIN_KEY_INDEX] and 
			!previousKeyStates[cast<u32>(key) - MIN_KEY_INDEX];
}

bool Window::keyJustReleased(Key key) const
{
	std::unique_lock lock(mut);
	return !keysHidden[cast<u32>(key) - MIN_KEY_INDEX] and
		!keysPressed[cast<u32>(key) - MIN_KEY_INDEX] and
		previousKeyStates[cast<u32>(key) - MIN_KEY_INDEX];
}

void Window::consumeKeyEvent(Key key)
{
	std::unique_lock lock(mut);
	keysHidden[cast<u32>(key) - MIN_KEY_INDEX] = true;

	std::remove_if(keyEventsInOrder.begin(), keyEventsInOrder.end(), [key](KeyEvent const& pkey) { return pkey.key == key; });
}

std::vector<KeyEvent> Window::getKeyEventsInOrder() const
{
	std::unique_lock lock(mut);
	return keyEventsInOrder;
}

bool Window::buttonPressed(MouseButton button) const
{
	std::unique_lock lock(mut);
	return (!mouseButtonHide[cast<u32>(button) - MIN_MOUSE_BUTTON_INDEX]) and mouseButtonStates[cast<u32>(button) - MIN_MOUSE_BUTTON_INDEX];
}

bool Window::buttonJustPressed(MouseButton button) const
{
	std::unique_lock lock(mut);
	return (!mouseButtonHide[cast<u32>(button) - MIN_MOUSE_BUTTON_INDEX]) and 
		mouseButtonStates[cast<u32>(button) - MIN_MOUSE_BUTTON_INDEX] and
		!previousMouseButtonStates[cast<u32>(button) - MIN_MOUSE_BUTTON_INDEX];
}

bool Window::buttonJustReleased(MouseButton button) const
{
	std::unique_lock lock(mut);
	return (!mouseButtonHide[cast<u32>(button) - MIN_MOUSE_BUTTON_INDEX]) and
		!mouseButtonStates[cast<u32>(button) - MIN_MOUSE_BUTTON_INDEX] and
		previousMouseButtonStates[cast<u32>(button) - MIN_MOUSE_BUTTON_INDEX];
}

void Window::consumeMouseButtonEvent(MouseButton button)
{
	std::unique_lock lock(mut);
	mouseButtonHide[cast<u32>(button) - MIN_MOUSE_BUTTON_INDEX] = true;
}

Vec2 Window::getCursorPos() const
{
	std::unique_lock lock(mut);
	return cursorPosition;
}

Vec2 Window::getPrevCursorPos() const
{
	std::unique_lock lock(mut);
	return previousCursorPosition;
}