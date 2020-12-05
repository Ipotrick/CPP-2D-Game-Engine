#pragma once

#include "UIElement.hpp"

class UIBar : public UIElement {
public:
	struct Parameters {
		// UIElement:
		UIAnchor anchor;
		Vec2 size{ 0.0f, 0.0f };
		std::function<void(UIElement*)> fn_update{ {} };
		std::function<bool(UIElement*)> fn_enableIf{ {} };
		// UIBar:
		float fill{ 0.0f };
		Vec4 emptyColor{ 0.0f, 0.0f, 0.0f, 1.0f };
		Vec4 fillColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	};

	UIBar(Parameters param = Parameters())
	{
		// UIElement:
		this->anchor = param.anchor;
		this->size = param.size;
		this->fn_update = param.fn_update;
		this->fn_enableIf = param.fn_enableIf;
		// UIBar:
		this->fill = param.fill;
		this->emptyColor = param.emptyColor;
		this->fillColor = param.fillColor;
	}

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