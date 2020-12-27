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

	virtual void draw(std::vector<Sprite>& buffer, UIContext context) override;

	void setHeadSize(const Vec2 size) { 
		this->headSize = size;
		bAutoHeadWidth = false;
	}
	void setHeadLength(const float f)
	{
		this->headSize.y = f;
	}
	void setHeadWidth(const float f)
	{
		this->headSize.x = f;
		bAutoHeadWidth = false;
	}
	Vec2 getHeadSize() const { return headSize; }

	void setBodyHeight(const float f) { 
		this->bodyHeight = f;
		this->bAutoBodyLength = false;
	}
	float getBodyHeight() const { return bodyHeight; }
	void setAutoBodyHeight(const bool b = true) { this->bAutoBodyLength = b; }
	bool autoBodyHeight() const { return bAutoBodyLength; }

	void setTitleFontSize(const Vec2 size) { this->textElement.fontSize = size; }
	UIAnchor& getTitleAnchor() { return textElement.textAnchor; }
	void setTitleColor(const Vec4 color) { this->textElement.color = color; }
	void setArrowScale(const float f) { this->arrowScale = clamp(f, 0.1f, 1.0f); }
	float getArrowScale() const { return arrowScale; }

	virtual void onRelease() override
	{
		bCollapsed = !bCollapsed;
		if (bCollapsed) {
			getChild()->disable();
		}
		else {
			getChild()->enable();
		}
	}

private:
	using UIClickable::setSize;

	UIText textElement;
	Vec4 borderColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	Vec4 fillColor{ 0.0f, 0.0f, 0.0f, 1.0f };
	Vec2 border{ standartBorder };
	Vec2 headSize{ 0.0f, 0.0f };
	float bodyHeight{ 0.0f };
	float arrowScale{ 0.65f };
	bool bAutoBodyLength{ true };
	bool bAutoHeadWidth{ true };
	bool bCollapsed{ true };
};