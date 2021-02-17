#pragma once

#include "UIFocusable.hpp"

class UIClickable : public UIFocusable {
public:
	virtual void disable() override
	{
		UIFocusable::disable();
		bPressed = false;
	}

	virtual void onClick() {}

	virtual void onHold() {}

	virtual void onRelease() {}

	bool bPressed{ false };
private:
};