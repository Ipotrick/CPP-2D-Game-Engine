#include "InputManager.hpp"

void InputManager::updateKeyStates(Window& window)
{
	oldKeyStates = newKeyStates;
	window.mut.lock();
	for (int key = MIN_KEY_INDEX; key < MAX_KEY_INDEX + 1; ++key) {
		newKeyStates[key] = glfwGetKey(window.glfwWindow, key);
	}
	window.mut.unlock();
}

bool InputManager::keyPressed(const KEY key, int focusKey)
{
	if (focus == focusKey || focusKey == GLOBAL_FOCUS)
		return newKeyStates[static_cast<int>(key)] == KEY_DOWN;
	else
		return KEY_UP;
}

bool InputManager::keyJustPressed(const KEY key, int focusKey)
{
	if (focus == focusKey || focusKey == GLOBAL_FOCUS)
		return newKeyStates[static_cast<int>(key)] == KEY_DOWN && oldKeyStates[static_cast<int>(key)] == KEY_UP;
	else
		return KEY_UP;
}

bool InputManager::keyReleased(const KEY key, int focusKey)
{
	if (focus == focusKey || focusKey == GLOBAL_FOCUS)
		return !keyPressed(key);
	else
		return KEY_UP;
}

bool InputManager::keyJustReleased(const KEY key, int focusKey)
{
	if (focus == focusKey || focusKey == GLOBAL_FOCUS)
		return newKeyStates[static_cast<int>(key)] == KEY_UP && oldKeyStates[static_cast<int>(key)] == KEY_DOWN;
	else
		return KEY_UP;
}
