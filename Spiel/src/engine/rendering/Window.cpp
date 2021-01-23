#include "Window.hpp"

#include <iostream>
#include <cassert>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

void keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods)
{
	Window* window = (Window*)glfwGetWindowUserPointer(win);
	if (action == GLFW_PRESS) {
		if (!window->keyStates[key - MIN_KEY_INDEX]) {
			window->keyEventsInOrder.push_back(KeyEvent{ cast<Key>(key), KeyEvent::Type::JustPressed });
		}
		window->keyEventsInOrder.push_back(KeyEvent{ cast<Key>(key), KeyEvent::Type::Pressed });
	}
	else if (action == GLFW_RELEASE && window->keyStates[key - MIN_KEY_INDEX]) {
		window->keyEventsInOrder.push_back(KeyEvent{ cast<Key>(key), KeyEvent::Type::JustReleased });
	}
}

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

	glfwSetKeyCallback(glfwWindow, keyCallback);
	glfwSetWindowUserPointer(glfwWindow, this);

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
		previousKeyStates[keyIndex] = keyStates[keyIndex];
		keyStates[keyIndex] = cast<bool>(glfwGetKey(glfwWindow, keyIndex + MIN_KEY_INDEX));
		keyHide[keyIndex] = false;
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

bool Window::keyPressed(Key key) const
{
	std::unique_lock lock(mut);
	return (!keyHide[cast<u32>(key) - MIN_KEY_INDEX]) and keyStates[cast<u32>(key) - MIN_KEY_INDEX];
}

bool Window::keyJustPressed(Key key) const
{
	std::unique_lock lock(mut);
	return !keyHide[cast<u32>(key) - MIN_KEY_INDEX] and 
			keyStates[cast<u32>(key) - MIN_KEY_INDEX] and 
			!previousKeyStates[cast<u32>(key) - MIN_KEY_INDEX];
}

bool Window::keyJustReleased(Key key) const
{
	std::unique_lock lock(mut);
	return !keyHide[cast<u32>(key) - MIN_KEY_INDEX] and
		!keyStates[cast<u32>(key) - MIN_KEY_INDEX] and
		previousKeyStates[cast<u32>(key) - MIN_KEY_INDEX];
}

void Window::consumeKeyEvent(Key key)
{
	std::unique_lock lock(mut);
	keyHide[cast<u32>(key) - MIN_KEY_INDEX] = true;

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
