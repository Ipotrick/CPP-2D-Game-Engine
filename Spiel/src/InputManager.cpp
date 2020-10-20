#include "InputManager.hpp"

InputManager::InputManager(Window& window)
	:window{ window }
{
	for (int i = MIN_KEY_INDEX; i < MAX_KEY_INDEX + 1; i++) {
		oldKeyStates[i] = GLFW_RELEASE;
	}
	for (int i = 0; i < 8; ++i) {
		oldButtonStates[i] = GLFW_RELEASE;
	}
	keyFocusStack.push_back(Focus::Standard);
	mouseFocusStack.push_back(Focus::Standard);
}

void InputManager::manualUpdate(const Camera& cam)
{
	if (!bManuallyUpdated) {
		inputUpdate(cam);
		bManuallyUpdated = true;
	}
	else {
		std::cerr << "WARNING: manualUpdate() SHOULD NEVER be called twice in a frame!\n";
	}
}

void InputManager::engineUpdate(const Camera& cam)
{
	if (!bManuallyUpdated) {
		inputUpdate(cam);
	}
	bManuallyUpdated = false;
}

void InputManager::inputUpdate(const Camera& cam)
{
	std::unique_lock lock(window.mut);
	oldKeyStates = newKeyStates;
	oldButtonStates = newButtonStates;
	prevMousePositionPixelSpace = mousePositionPixelSpace;
	prevMousePositionUniformWindowSpace = mousePositionUniformWindowSpace;
	prevMousePositionWindowSpace = mousePositionWindowSpace;
	prevMousePositionWorldSpace = mousePositionWorldSpace;
	glfwPollEvents();	// update key and button states

	// when the focus is Out, the keys are getting set to released
	if (getFocus() != Focus::Out) {
		for (int key = MIN_KEY_INDEX; key < MAX_KEY_INDEX + 1; ++key) {
			newKeyStates[key] = glfwGetKey(window.glfwWindow, key);
		}
	}
	else {
		for (int key = MIN_KEY_INDEX; key < MAX_KEY_INDEX + 1; ++key) {
			newKeyStates[key] = GLFW_RELEASE;
		}
	}

	// when the focus is Out, the buttons are getting set to released and the cursorposition is not updated anymore
	if (getMouseFocus() != Focus::Out) {
		for (int button = 0; button < 8; ++button) {
			newButtonStates[button] = glfwGetMouseButton(window.glfwWindow, button);
		}
		Vec2 size = { static_cast<float>(window.width), static_cast<float>(window.height) };
		double xPos, yPos;
		glfwGetCursorPos(window.glfwWindow, &xPos, &yPos);
		mousePositionWindowSpace = { (float)xPos / size.x * 2.0f - 1.f, -(float)yPos / size.y * 2.0f + 1.f };
		mousePositionUniformWindowSpace = { mousePositionWindowSpace.x * size.x / size.y, mousePositionWindowSpace.y };
		mousePositionPixelSpace = { (float)xPos, (float)window.height - (float)yPos };
		mousePositionWorldSpace = cam.windowToWorld(mousePositionWindowSpace);
	}
	else {
		for (int button = 0; button < 8; ++button) {
			newButtonStates[button] = GLFW_RELEASE;
		}
	}
}

bool InputManager::keyPressed(const Key key, const Focus focus) const
{
	if (keyFocusGuard(focus)) {
		return newKeyStates[static_cast<int>(key)] == GLFW_PRESS;
	}
	else {
		return false;
	}
}

bool InputManager::keyJustPressed(const Key key, const Focus focus) const
{
	if (keyFocusGuard(focus)) {
		return newKeyStates[static_cast<int>(key)] == GLFW_PRESS && oldKeyStates[static_cast<int>(key)] == GLFW_RELEASE;
	}
	else {
		return false;
	}
}

bool InputManager::keyReleased(const Key key, const Focus focus) const
{
	if (keyFocusGuard(focus)) {
		return !keyPressed(key);
	}
	else {
		return false;
	}
}

bool InputManager::keyJustReleased(const Key key, const Focus focus) const
{
	if (keyFocusGuard(focus)) {
		return newKeyStates[static_cast<int>(key)] == GLFW_RELEASE && oldKeyStates[static_cast<int>(key)] == GLFW_PRESS;
	}
	else {
		return false;
	}
}

bool InputManager::buttonPressed(const Button but, Focus focus) const
{
	if (mouseFocusGuard(focus)) {
		return newButtonStates[static_cast<int>(but)] == GLFW_PRESS;
	}
	else {
		return false;
	}
}

bool InputManager::buttonJustPressed(const Button but, Focus focus) const
{
	if (mouseFocusGuard(focus)) {
		return newButtonStates[static_cast<int>(but)] == GLFW_PRESS && oldButtonStates[static_cast<int>(but)] == GLFW_RELEASE;
	}
	else {
		return false;
	}
}

bool InputManager::buttonReleased(const Button but, Focus focus) const
{
	if (mouseFocusGuard(focus)) {
		return !buttonPressed(but, focus);
	}
	else {
		return false;
	}
}

bool InputManager::buttonJustReleased(const Button but, Focus focus) const
{
	if (mouseFocusGuard(focus)) {
		return newButtonStates[static_cast<int>(but)] == GLFW_RELEASE && oldButtonStates[static_cast<int>(but)] == GLFW_PRESS;
	}
	else {
		return false;
	}
}

Vec2 InputManager::getMousePosition(const RenderSpace renderSpace) const
{
	switch (renderSpace) {
	case RenderSpace::PixelSpace:
		return mousePositionPixelSpace;
	case RenderSpace::WindowSpace:
		return mousePositionWindowSpace;
	case RenderSpace::UniformWindowSpace:
		return mousePositionWindowSpace;
	case RenderSpace::WorldSpace:
		return mousePositionWorldSpace;
	default:
		assert(false); // when this assert fails, you did not update this function for altered RenderSpace enum types >:(
		return { 0.0f, 0.0f };
	}
}

Vec2 InputManager::getPrevMousePosition(RenderSpace renderSpace) const
{
	switch (renderSpace) {
	case RenderSpace::PixelSpace:
		return prevMousePositionPixelSpace;
	case RenderSpace::WindowSpace:
		return prevMousePositionWindowSpace;
	case RenderSpace::UniformWindowSpace:
		return prevMousePositionWindowSpace;
	case RenderSpace::WorldSpace:
		return prevMousePositionWorldSpace;
	default:
		assert(false); // when this assert fails, you did not update this function for altered RenderSpace enum types >:(
		return { 0.0f, 0.0f };
	}
}
