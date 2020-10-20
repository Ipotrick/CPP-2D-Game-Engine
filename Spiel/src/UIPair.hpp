#pragma once

#include "UIElement.hpp"

class UIPair : public UIElement {
public:
	UIPair() = default;

	UIPair (UIElement* first, UIElement* second)
		:first{ first }, second{ second }
	{}

	/*
	* the first element is either drawn on the left or on top depending on the horizontal setting
	* per default the horizontal setting is set to false
	*/
	virtual void draw(std::vector<Drawable>& buffer, UIContext context) override;

	virtual void postUpdate()
	{
		first->postUpdate();
		second->postUpdate();
	}

	virtual void destroy() override
	{
		UIElement::destroy();
		first->destroy();
		second->destroy();
	}

	virtual void enable() override
	{
		UIElement::enable();
		first->enable();
		second->enable();
	}

	virtual void disable() override
	{
		UIElement::disable();
		first->disable();
		second->disable();
	}

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
	void setHorizontal(const bool b = true)
	{
		this->bHorizonal = b;
	}
	void setAutoSize(const bool b = true)
	{
		this->bAutoSize = b;
	}
private:
	bool bAutoSize{ true };
	bool bHorizonal{ false };
	UIElement* first{ nullptr };
	UIElement* second{ nullptr };
};