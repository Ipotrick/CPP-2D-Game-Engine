#pragma once

#include "UIElement.hpp"
#include "UIClickable.hpp"

class UIButton : public UIClickable {
public:
	virtual void draw(std::vector<Sprite>& buffer, UIContext context) override;

	virtual void onEnter() override
	{
		onEnterFn(this);
	}

	virtual void onHover() override
	{
		onHoverFn(this);
	}

	virtual void onLeave() override
	{
		onLeaveFn(this);
	}

	void setEnterFn(std::function<void(UIFocusable*)> fn)
	{
		this->onEnterFn = fn;
	}

	void setHoverFn(std::function<void(UIFocusable*)> fn)
	{
		this->onHoverFn = fn;
	}

	void setLeaveFn(std::function<void(UIFocusable*)> fn)
	{
		this->onLeaveFn = fn;
	}

	virtual void onClick() override
	{
		onClickFn(this);
	}

	virtual void onHold() override
	{
		onHoldFn(this);
	}

	virtual void onRelease() override
	{
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

	float border{ 2.0f };
	Vec4 borderColor{ 0.0f, 0.0f, 0.0f, 1.0f };
	Vec4 innerReleasedColor{ 0.8f, 0.8f, 0.8f, 1.0f };
	Vec4 innerPressedColor{ 0.0f, 0.0f, 0.0f, 1.0f };
private:
	std::function<void(UIFocusable*)> onEnterFn{	[](UIFocusable* me) {} };
	std::function<void(UIFocusable*)> onHoverFn{	[](UIFocusable* me) {} };
	std::function<void(UIFocusable*)> onLeaveFn{	[](UIFocusable* me) {} };
	std::function<void(UIClickable*)> onClickFn{	[](UIClickable* me) {} };
	std::function<void(UIClickable*)> onHoldFn{		[](UIClickable* me) {} };
	std::function<void(UIClickable*)> onReleaseFn{	[](UIClickable* me) {} };
};