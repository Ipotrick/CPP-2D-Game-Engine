#pragma once

#include "UIElement.hpp"
#include "InputFocus.hpp"

class UIFocusable : public UIElement {
public:

	virtual void disable() override
	{
		UIElement::disable();
	}

	virtual void onEnter() {}

	virtual void onHover() {}

	virtual void onLeave() {}

	virtual const UIContext& getFocusArea() const final
	{
		return lastDrawArea;
	}

	void setFocusable(const bool b)
	{
		this->bFocusable = b;
	}

	bool isFocusable() const 
	{ 
		return this->bFocusable;
	}

	Focus hoverFocus{ Focus::UI };
	bool bHoveredOver{ false };
protected:
	/*
	* One must set this struct in the draw function
	*/
	UIContext lastDrawArea;
	bool bFocusable{ true };
};