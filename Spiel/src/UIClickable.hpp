#pragma once

#include "UIFocusable.hpp"

class UIClickable : public UIFocusable {
public:
	virtual void disable() override
	{
		UIFocusable::disable();
		bPressed = false;
	}

	virtual void onClick() final
	{
		bPressed = true;
		onClickFn(this);
	}

	virtual void onHold() final
	{
		bPressed = true;
		onHoldFn(this);
	}

	virtual void onRelease() final
	{
		bPressed = false;
		onReleaseFn(this);
	}
	
	virtual void setClickFn(std::function<void(UIClickable*)> fn) final 
	{
		this->onClickFn = fn;
	}

	virtual void setHoldFn(std::function<void(UIClickable*)> fn) final 
	{
		this->onHoldFn = fn;
	}

	virtual void setReleaseFn(std::function<void(UIClickable*)> fn) final
	{
		this->onReleaseFn = fn;
	}

	bool bPressed{ false };
private:
	std::function<void(UIClickable*)> onClickFn{ [](UIClickable* me) {} };
	std::function<void(UIClickable*)> onHoldFn{ [](UIClickable* me) {} };
	std::function<void(UIClickable*)> onReleaseFn{ [](UIClickable* me) {} };
};