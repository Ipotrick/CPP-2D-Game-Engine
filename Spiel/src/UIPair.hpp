#pragma once

#include "UIElement.hpp"

class UIPair : public UIElement {
public:
	/*
	* the first element is either drawn on the left or on top depending on the horizontal setting
	* per default the horizontal setting is set to false
	*/
	virtual void draw(std::vector<Drawable>& buffer, UIContext context) override;

	void setPair(UIElement* first, UIElement* second)
	{
		this->first = first;
		this->second = second;
	}
	void setFirst(UIElement* element)
	{
		this->first = element;
	}
	void setSecond(UIElement* element)
	{
		this->second = element;
	}
	void setHorizontal(const bool b)
	{
		this->bHorizonal = b;
	}
private:
	bool bHorizonal{ false };
	UIElement* first{ nullptr };
	UIElement* second{ nullptr };
};