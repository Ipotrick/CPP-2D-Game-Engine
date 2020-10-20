#pragma once

#include "UIElement.hpp"

class UISingleParent {
public:
	/*
	* if this function is overriden, the Base Function (UISingleParent::destroy) MUST still be called
	virtual void destroy() override
	{
		UIElement::destroy();
		if (hasChild()) {
			destroyChild();
		}
	}
	*/

	/*
	* if this function is overriden, the Base Function (UISingleParent::enable) MUST still be called
	virtual void enable() override
	{
		UIElement::enable();
		if (hasChild()) {
			enableChild();
		}
	}
	*/

	/*
	* if this function is overriden, the Base Function (UISingleParent::destdisableroy) MUST still be called
	virtual void disable() override
	{
		UIElement::disable();
		if (hasChild()) {
			disableChild();
		}
	}
	*/

	/*
	* if this function is overriden, the Base Function (UISingleParent::updateSize) MUST still be called
	virtual void postUpdate() override
	{
		if (hasChild()) {
			getChild()->postUpdate();
		}
	}
	*/

	void addChild(UIElement* child)
	{
		assert(this->child == nullptr);
		this->child = child;
	}
	UIElement* getChild()
	{
		assert(this->child != nullptr, "no child to return!");
		return this->child;
	}
	/*
	* cleares child without destroying it
	*/
	void remChild()
	{
		assert(this->child != nullptr, "no child to remove!");
		this->child = nullptr;
	}
	/*
	* cleares child and calling destroy on child
	*/
	void destroyChild()
	{
		assert(this->child != nullptr, "no child to destroy!");
		child->destroy();
		child = nullptr;
	}
	bool hasChild() const
	{
		return this->child != nullptr;
	}
	void enableChild()
	{
		assert(child != nullptr);
		child->enable();
	}
	void disableChild()
	{
		assert(child != nullptr);
		child->disable();
	}
	bool isChildEnabled() const
	{
		assert(child != nullptr);
		return child->isEnabled();
	}
private:
	UIElement* child{ nullptr };
};

