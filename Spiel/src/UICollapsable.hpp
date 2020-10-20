#pragma once

#include "UIClickable.hpp"
#include "UISingleParent.hpp"
#include "UIText.hpp"
#include "DrawFrame.hpp"

class UICollapsable : public UIClickable, public UISingleParent {
public:
	UICollapsable()
	{
		setSize({ 20.0f, 100.0f });
		textElement.textAnchor.setCenterHorizontal();
		textElement.textAnchor.setCenterVertical();
	}

	UICollapsable(const char* title, SmallTextureRef font, Vec2 headerSize = Vec2{100,20})
		:textElement{ title, font }, headSize{ headerSize }
	{
		setSize(headerSize);
		textElement.textAnchor.setCenterHorizontal();
		textElement.textAnchor.setCenterVertical();
	}

	virtual void destroy() override
	{
		UIElement::destroy();
		if (hasChild()) {
			destroyChild();
		}
	}

	virtual void enable() override
	{
		UIElement::enable();
		if (hasChild()) {
			enableChild();
		}
	}

	virtual void disable() override
	{
		UIElement::disable();
		if (hasChild()) {
			disableChild();
		}
	}

	virtual void postUpdate() override
	{
		if (hasChild()) {
			getChild()->postUpdate();
		}
	}

	virtual void draw(std::vector<Drawable>& buffer, UIContext context) override;

	void setHeadSize(const Vec2 size) { this->headSize = size; }
	Vec2 getHeadSize() const { return headSize; }

	void setBodyHeight(const float f) { this->bodyHeight = f; }
	float getBodyHeight() const { return bodyHeight; }
	void setAutoBodyHeight(const bool b = true) { this->bAutoBodyHeight = b; }
	bool autoBodyHeight() const { return bAutoBodyHeight; }

	void setTitleFontSize(const Vec2 size) { this->textElement.fontSize = size; }
	UIAnchor& getTitleAnchor() { return textElement.textAnchor; }
	void setTitleColor(const Vec4 color) { this->textElement.color = color; }
private:
	using UIClickable::setSize;
	Vec4 borderColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	Vec4 fillColor{ 0.0f, 0.0f, 0.0f, 1.0f };
	Vec2 border{ standartBorder };

	bool bAutoBodyHeight{ true };
	float bodyHeight{ 0.0f };
	Vec2 headSize{ 0.0f, 0.0f };

	UIText textElement;
};