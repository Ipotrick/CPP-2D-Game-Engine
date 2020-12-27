#pragma once

#include <array>
#include <type_traits>
#include <vector>

#include "Input.hpp"
#include "../rendering/Window.hpp"
#include "../math/Vec2.hpp"
#include "InputFocus.hpp"

class InputManager {
public:
	InputManager(Window& window);
	bool keyPressed(const Key key, Focus focus = Focus::Standard) const;
	/*
	* returns if the key is pressed in the current frame but was released in the previous frame.
	*/
	bool keyJustPressed(const Key key, Focus focus = Focus::Standard) const;
	bool keyReleased(const Key key, Focus focus = Focus::Standard) const;
	/*
	* returns if the key is released in the current frame but was pressed in the previous frame.
	*/
	bool keyJustReleased(const Key key, Focus focus = Focus::Standard) const;


	bool buttonPressed(const Button but, Focus focus = Focus::Standard) const;
	/*
	* returns if the button is pressed in the current frame but was released in the previous frame.
	*/
	bool buttonJustPressed(const Button but, Focus focus = Focus::Standard) const;
	bool buttonReleased(const Button but, Focus focus = Focus::Standard) const;
	/*
	* returns if the button is released in the current frame but was pressed in the previous frame.
	*/
	bool buttonJustReleased(const Button but, Focus focus = Focus::Standard) const;


	/**
	 * \return vector with x,y position of the mouse in windowspace
	 */
	Vec2 getMousePosition() const;

	/**
	 * \return vector with x,y position of the mouse in windowspace, of the previous input update (most likely last frame)
	 */
	Vec2 getPrevMousePosition() const;

	/**
	 * Sets the current focus of the keyboard input system.
	 * The set focus sits on a stack so it remembers what state it was in before.
	 * calling returnFocus will pop revert the focus to the previous focus.
	 * 
	 * \param focus that the keyboard input is set to.
	 */
	void takeFocus(Focus focus)
	{
		this->keyFocusStack.push_back(focus);
	}

	/**
	 * \return current input focus of the keyboard
	 */
	Focus getFocus() const
	{
		return keyFocusStack.back();
	}

	/**
	 * reverts focus of keyboard input to the focus it was before taking focus.
	 */
	void returnFocus()
	{
		keyFocusStack.pop_back();
	}

	/**
	 * Sets the current focus of the mouse input system.
	 * The set focus sits on a stack so it remembers what state it was in before.
	 * calling returnFocus will pop revert the focus to the previous focus.
	 *
	 * \param focus that the mouse input is set to.
	 */
	void takeMouseFocus(Focus focus)
	{
		this->mouseFocusStack.push_back(focus);
	}

	/**
	 * \return current input focus of the mouse
	 */
	Focus getMouseFocus() const
	{
		return mouseFocusStack.back();
	}
	/**
	 * reverts focus of mouse input to the focus it was before taking focus.
	 */
	void returnMouseFocus()
	{
		this->mouseFocusStack.pop_back();
	}

	/**
	 * You can call this function ONCE per frame to update the keystates.
	 * If you do not call this function the engine will update the keystates itself after each frame.
	 */
	void manualUpdate();
private:
	bool keyFocusGuard(const Focus focus) const
	{
		return getFocus() == focus || focus == Focus::Global;
	}

	bool mouseFocusGuard(const Focus focus) const
	{
		return getMouseFocus() == focus || focus == Focus::Global;
	}

	friend class EngineCore;
	void engineUpdate();	// this function is ment to be called by the engine once a frame	

	void inputUpdate();

	Window& window;
	bool bManuallyUpdated{ false };
	std::vector<Focus> keyFocusStack;
	std::vector<Focus> mouseFocusStack;
	std::array<char, MAX_KEY_INDEX + 1> oldKeyStates;
	std::array<char, MAX_KEY_INDEX + 1> newKeyStates;
	std::array<char, 8> oldButtonStates;
	std::array<char, 8> newButtonStates;
	Vec2 mousePositionWindowSpace{ 0.0f, 0.0f };
	Vec2 prevMousePositionWindowSpace{ 0.0f, 0.0f };
};