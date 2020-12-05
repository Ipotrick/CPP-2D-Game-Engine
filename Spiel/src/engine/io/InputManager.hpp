#pragma once

#include <array>
#include <type_traits>
#include <vector>

#include "Input.hpp"
#include "../rendering/Window.hpp"
#include "../math/Vec2.hpp"
#include "../rendering/RenderSpace.hpp"
#include "InputFocus.hpp"
#include "../rendering/Camera.hpp"

static inline std::string focusToString(Focus focus)
{
	switch (focus) {
	case Focus::Out:
		return "Out";
	case Focus::Global:
		return "Gloabal";
	case Focus::Standard:
		return "Standard";
	case Focus::WriteText:
		return "WriteText";
	case Focus::UI:
		return "Menu";
	default:
		assert(false);
		return "";
	}
}

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


	Vec2 getMousePosition(RenderSpace renderSpace = RenderSpace::WindowSpace) const;
	Vec2 getPrevMousePosition(RenderSpace renderSpace = RenderSpace::WindowSpace) const;

	void takeFocus(Focus focus)
	{
		this->keyFocusStack.push_back(focus);
	}
	Focus getFocus() const
	{
		return keyFocusStack.back();
	}
	void returnFocus()
	{
		keyFocusStack.pop_back();
	}

	void takeMouseFocus(Focus focus)
	{
		this->mouseFocusStack.push_back(focus);
	}
	Focus getMouseFocus() const
	{
		return mouseFocusStack.back();
	}
	void returnMouseFocus()
	{
		this->mouseFocusStack.pop_back();
	}

	/*
	* you can call this function ONCE per frame to update the keystates.
	* If you do not call this function the engine will update the keystates itself after each frame.
	*/
	void manualUpdate(const Camera& cam);
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
	void engineUpdate(const Camera& cam);	// this function is ment to be called by the engine once a frame	

	void inputUpdate(const Camera& cam);

	Window& window;
	bool bManuallyUpdated{ false };
	std::vector<Focus> keyFocusStack;
	std::vector<Focus> mouseFocusStack;
	std::array<char, MAX_KEY_INDEX + 1> oldKeyStates;
	std::array<char, MAX_KEY_INDEX + 1> newKeyStates;
	std::array<char, 8> oldButtonStates;
	std::array<char, 8> newButtonStates;
	Vec2 mousePositionWindowSpace{ 0.0f, 0.0f };
	Vec2 mousePositionUniformWindowSpace{ 0.0f, 0.0f };
	Vec2 mousePositionPixelSpace{ 0.0f, 0.0f };
	Vec2 mousePositionWorldSpace{ 0.0f, 0.0f };
	Vec2 prevMousePositionWindowSpace{ 0.0f, 0.0f };
	Vec2 prevMousePositionUniformWindowSpace{ 0.0f, 0.0f };
	Vec2 prevMousePositionPixelSpace{ 0.0f, 0.0f };
	Vec2 prevMousePositionWorldSpace{ 0.0f, 0.0f };
};