#pragma once

#include "UIElement.hpp"

class UIBar : public UIElement {
public:
	UIBar() = default;
	UIBar(Vec4 empty, Vec4 fill)
		:emptyColor{ empty }, fillColor{ fill }
	{ }
	virtual void draw(std::vector<Drawable>& buffer, UIContext context) override;

	void setFill(const float fill) { this->fill = clamp(fill, 0.0f, 1.0f); }
	float getFill() const { return this->fill; }
	void setEmptyColor(const Vec4& color) { this->emptyColor = color; }
	const Vec4& getEmptyColor() const { return emptyColor; }
	void setFillColor(const Vec4& color) { this->fillColor = color; }
	const Vec4& getFillColor() const { return fillColor; }
private:
	float fill{ 0.0f };
	Vec4 emptyColor{ 0.0f, 0.0f, 0.0f, 1.0f };
	Vec4 fillColor{ 1.0f, 1.0f, 1.0f, 1.0f };
};