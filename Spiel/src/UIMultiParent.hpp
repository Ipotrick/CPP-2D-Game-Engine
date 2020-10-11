#pragma once

#include "UIElement.hpp"
#include <boost/container/static_vector.hpp>

template<size_t CAPACITY>
class UIMultiParent : public UIElement {
public:
	/*
	* if this function is overriden, the Base Function (UIMultiParent::destroy) MUST still be called
	*/
	virtual void destroy() override
	{
		UIElement::destroy();
		for (auto& child : children) {
			child->destroy();
		}
	}

	/*
	* if this function is overriden, the Base Function (UIMultiParent::enable) MUST still be called
	*/
	virtual void enable() override
	{
		UIElement::enable();
		for (auto& child : children) {
			child->enable();
		}
	}

	/*
	* if this function is overriden, the Base Function (UIMultiParent::disable) MUST still be called
	*/
	virtual void disable() override
	{
		UIElement::disable();
		for (auto& child : children) {
			child->disable();
		}
	}

	/*
	* if this function is overriden, the Base Function (UIMultiParent::updateSize) MUST still be called
	*/
	virtual void postUpdate() override
	{
		for (auto& child : children) {
			child->postUpdate();
		}
	}

	void addChild(UIElement* child)
	{
		assert(children.size() < CAPACITY);
		children.push_back(child);
	}

	/*
	* cleares child without destroying it
	*/
	void remChild(const int index)
	{
		children.erase(children.begin() + index);
	}

	UIElement* getChild(const int index)
	{
		return children.at(index);
	}

	const UIElement* getChild(const int index) const
	{
		return getChild(index);
	}

	/*
	* calls destroy on child
	*/
	void destroyChild(const int index)
	{
		getChild(index)->destroy();
	}

	void enableChild(const int index)
	{
		getChild(index)->enable();
	}

	void disableChild(const int index)
	{
		getChild(index)->disable();
	}

	bool isChildEnabled(const int index) const
	{
		getChild(index)->isEnabled();
	}

	bool hasChild() const
	{
		return !children.empty();
	}

	size_t childCount() const { return children.size(); }

protected:
	boost::container::static_vector<UIElement*, CAPACITY> children;
};