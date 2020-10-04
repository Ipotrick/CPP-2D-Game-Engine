#include <array>

#include "Input.hpp"
#include "Window.hpp"

inline static constexpr int GLOBAL_FOCUS = -1;
inline static constexpr int STANDART_FOCUS = 0;
inline static constexpr int TEXT_INPUT_FOCUS = 1;
inline static constexpr int MAIN_MENU_FOCUS = 2;
inline static constexpr int SUB_MENU_FOCUS = 3;

class InputManager {
public:
	InputManager()
	{
		for (int i = MIN_KEY_INDEX; i < MAX_KEY_INDEX + 1; i++) {
			oldKeyStates[i] = KEY_UP;
			newKeyStates[i] = KEY_UP;
		}
	}
	bool keyPressed(const KEY key, int focusKey = STANDART_FOCUS);
	bool keyJustPressed(const KEY key, int focusKey = STANDART_FOCUS);
	bool keyReleased(const KEY key, int focusKey = STANDART_FOCUS);
	bool keyJustReleased(const KEY key, int focusKey = STANDART_FOCUS);
	void setFocus(int focusKey)
	{
		focus = focusKey;
	}
	int getFocus() const
	{
		return focus;
	}

private:
	int focus{ 0 };

	friend class Engine;
	void updateKeyStates(Window& window);
	std::array<char, MAX_KEY_INDEX + 1> oldKeyStates;
	std::array<char, MAX_KEY_INDEX + 1> newKeyStates;
};