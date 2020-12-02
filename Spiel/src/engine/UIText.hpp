#pragma once

#include "UIElement.hpp"

class UIText : public UIElement {
public:

	UIText(std::string_view str, SmallTextureRef texRef)
		:text{ str }, fontTexture{ texRef }
	{
	}
	
	UIText(const char* str, SmallTextureRef texRef)
		:text{ str }, fontTexture{ texRef }
	{

	}

	UIText(const char* str, SmallTextureRef texRef, std::function<void(UIElement*)>&& updateFn)
		:text{ str }, fontTexture{ texRef }
	{
		setUpdateFn(updateFn);
	}

	UIText() {}

	virtual void draw(std::vector<Drawable>& buffer, UIContext context) override;

	void setSize(const Vec2 size)
	{
		this->bAutoSize = false;
		UIElement::setSize(size);
	}

	void setAutoSize(const bool b = true) { this->bAutoSize = b; }
	bool isAutoSized() const { return bAutoSize; }

	std::string text{ "" };
	SmallTextureRef fontTexture;
	Vec2 fontSize{ 17.0f / 2.0f, 17.0f };
	Vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
	UIAnchor textAnchor;
private:
	bool bAutoSize{ true };
	void calculateTextSize();
	Vec2 textSize{ 0.0f, 0.0f };
};