#pragma once

#include "UIElement.hpp"
#include "InputFocus.hpp"

class UIFocusable : public UIElement {
public:

	virtual void disable() override
	{
		UIElement::disable();
	}

	virtual void onEnter() final
	{
		bHoveredOver = true;
		onEnterFn(this);
	}

	virtual void onHover() final
	{
		bHoveredOver = true;
		onHoverFn(this);
	}

	virtual void onLeave() final
	{
		bHoveredOver = false;
		onLeaveFn(this);
	}

	virtual void setEnterFn(std::function<void(UIFocusable*)> fn) final
	{
		this->onEnterFn = fn;
	}

	virtual void setHoverFn(std::function<void(UIFocusable*)> fn) final
	{
		this->onHoverFn = fn;
	}

	virtual void setLeaveFn(std::function<void(UIFocusable*)> fn) final
	{
		this->onLeaveFn = fn;
	}

	virtual const UIContext& getLastDrawArea() const final
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
private:
	std::function<void(UIFocusable*)> onEnterFn{ [](UIFocusable* me) {} };
	std::function<void(UIFocusable*)> onHoverFn{ [](UIFocusable* me) {} };
	std::function<void(UIFocusable*)> onLeaveFn{ [](UIFocusable* me) {} };

};