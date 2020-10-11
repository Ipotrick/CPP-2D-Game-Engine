#pragma once

#include <functional>
#include <optional>

#include "UIContext.hpp"
#include "UIAnchor.hpp"

class UIElement {
public:
	virtual void draw(std::vector<Drawable>& buffer, UIContext context) = 0;

	virtual void destroy() {
		bDestroyed = true;
	}
	virtual bool isDestroyed() const final { return bDestroyed; }

	virtual void update() final
	{
		if (hasUpdateFn()) {
			fn_update(this);
		}
	}
	virtual bool hasUpdateFn() const final
	{
		return static_cast<bool>(fn_update);
	}
	virtual void setUpdateFn(std::function<void(UIElement*)> fn) final
	{
		fn_update = fn;
	}
	virtual std::function<void(UIElement*)> getUpdateFn() const final
	{
		return fn_update;
	}

	virtual void enable() { bEnable = true; }
	virtual void disable() { bEnable = false; }
	virtual bool isEnabled() const { return this->bEnable; }

	virtual void setSize(Vec2 size) final
	{
		this->size = std::move(size);
	}
	virtual Vec2 getSize() const final
	{
		return this->size;
	}
	/*
	* This function is called recursively for all frames and their elements AFTER the update call to all frames and elements
	*/
	virtual void postUpdate() {}

	UIAnchor anchor;
protected:
	Vec2 size{ 0.0f, 0.0f };
private:
	bool bDestroyed{ false };
	bool bEnable{ true };

	std::function<void(UIElement*)> fn_update{ {} };
};