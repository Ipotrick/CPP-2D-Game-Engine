#pragma once

#include "UIElement.hpp"

class UISeperator : public UIElement {
public:
	UISeperator()
	{
		anchor.setCenterHorizontal();
		anchor.setCenterVertical();
	}
	virtual void draw(std::vector<Sprite>& buffer, UIContext context) override;

	void setHorizontal(const bool b = true) { this->bHorizontal = b; }
	bool isHorizontal() const { return bHorizontal; }

	void setThiccness(const float t) { this->thiccnes = t; }
	float getThiccness() const { return thiccnes; }

	void setAutoWidth(const bool b = true) { this->bAutoWidth = b; }
	bool autoWidth() const { return bAutoWidth; }

	void setWidth(const float w)
	{
		this->bAutoWidth = false;
		this->width = w;
	}
	float getWidth() const { return width; }

	void setColor(const Vec4 color) { this->color = color; }
	Vec4 getColor() const { return color; }
private:
	Vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
	bool bHorizontal{ false };
	bool bAutoWidth{ true };
	float width{ 0.0f };
	float thiccnes{ standartBorder.x };
};