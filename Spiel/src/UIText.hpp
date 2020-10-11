#pragma once

#include "UIElement.hpp"

class UIText : public UIElement {
public:

	UIText(std::string_view str, SmallTextureRef texRef)
		:text{ str }, fontTexture{ texRef }
	{

	}

	UIText() {}

	virtual void draw(std::vector<Drawable>& buffer, UIContext context) override;

	virtual void postUpdate();

	std::string text{ "" };
	SmallTextureRef fontTexture;
	Vec2 borderDist{ 0.0f, 0.0f };
	Vec2 fontSize{ 1.0f, 1.0f };
	Vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
	UIAnchor textAnchor;
private:
	Vec2 textSize{ 0.0f, 0.0f };
};