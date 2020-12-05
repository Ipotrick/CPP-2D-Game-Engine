#pragma once

#include "UIElement.hpp"
#include "../io/InputFocus.hpp"

class UIFocusable : public UIElement {
public:

	struct Parameters {
		// UIElement:
		UIAnchor anchor;
		Vec2 size{ 0.0f, 0.0f };
		std::function<void(UIElement*)> fn_update{ {} };
		std::function<bool(UIElement*)> fn_enableIf{ {} };
		// UIFocusable:
		bool focusable = true;
	};

	UIFocusable(Parameters param = Parameters())
	{ 
		this->size = param.size;
		this->anchor = param.anchor;
		this->fn_update = param.fn_update;
		this->fn_enableIf = param.fn_enableIf;
		this->bFocusable = param.focusable;
	}

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