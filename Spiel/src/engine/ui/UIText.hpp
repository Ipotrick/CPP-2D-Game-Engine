#pragma once

#include "UIElement.hpp"

class UIText : public UIElement {
public:
	struct Parameters {
		// UIElement:
		UIAnchor anchor;
		Vec2 size{ 0.0f, 0.0f };
		std::function<void(UIElement*)> fn_update{ {} };
		std::function<bool(UIElement*)> fn_enableIf{ {} };
		// UIText:
		UIAnchor textAnchor;
		std::string text{ "" };
		TextureRef fontTexture;
		Vec2 fontSize{ 17.0f / 2.0f, 17.0f };
		Vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
	};
	
	UIText(Parameters param = Parameters())
	{
		// UIElement:
		this->anchor = param.anchor;
		this->size = param.size;
		this->fn_update = param.fn_update;
		this->fn_enableIf = param.fn_enableIf;
		// UIText:
		this->textAnchor = param.textAnchor;
		this->text = param.text;
		this->fontTexture = param.fontTexture;
		this->fontSize = param.fontSize;
		this->color = param.color;
	}

	UIText(const char* str, TextureRef texRef, std::function<void(UIElement*)>&& updateFn = {})
		:text{ str }, fontTexture{ texRef }
	{
		setUpdateFn(updateFn);
	}

	virtual void draw(std::vector<Sprite>& buffer, UIContext context) override;

	void setSize(const Vec2 size)
	{
		this->bAutoSize = false;
		UIElement::setSize(size);
	}

	void setAutoSize(const bool b = true) { this->bAutoSize = b; }
	bool isAutoSized() const { return bAutoSize; }

	std::string text{ "" };
	TextureRef fontTexture;
	Vec2 fontSize{ 17.0f / 2.0f, 17.0f };
	Vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
	UIAnchor textAnchor;
private:
	bool bAutoSize{ true };
	void calculateTextSize();
	Vec2 textSize{ 0.0f, 0.0f };
};