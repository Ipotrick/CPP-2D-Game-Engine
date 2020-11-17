#pragma once

#include <functional>
#include <optional>

#include "UIContext.hpp"
#include "UIAnchor.hpp"

static constexpr Vec2 standartBorder{ 1.0f, 1.0f };

class UIElement {
public:
	virtual void draw(std::vector<Drawable>& buffer, UIContext context) = 0;

	virtual void destroy()
	{
		bDestroyed = true;
	}
	bool isDestroyed() const { return bDestroyed; }

	void update()
	{
		if (!isDestroyed()) {
			if (hasDestroyIfFn() && destroyIfFn(this)) {
				destroy();
			}
			else {
				if (hasEnableIfFn()) {
					if (enableIfFn(this)) {
						enable();
					}
					else {
						disable();
					}
				}
				if (hasUpdateFn()) {
					fn_update(this);
				}
			}
		}
	}
	virtual bool hasUpdateFn() const final
	{
		return static_cast<bool>(fn_update);
	}
	virtual void setUpdateFn(const std::function<void(UIElement*)>& fn) final
	{
		fn_update = fn;
	}
	virtual void setUpdateFn(std::function<void(UIElement*)>&& fn) final
	{
		fn_update = std::move(fn);
	}

	virtual void enable() { bEnable = true; }
	virtual void disable() { bEnable = false; }
	virtual bool isEnabled() const { return this->bEnable; }

	virtual void setSize(Vec2 size)
	{
		this->size = std::move(size);
	}
	virtual Vec2 getSize() const
	{
		return this->size;
	}

	/*
	* the destroyIf function is called before the update function.
	*/
	void setDestroyIfFn(std::function<bool(UIElement*)> f)
	{
		this->destroyIfFn = f;
	}
	bool hasDestroyIfFn() const
	{
		return static_cast<bool>(destroyIfFn);
	}

	/*
	* enables element when function returns true
	*/
	void setEnableIfFn(std::function<bool(UIElement*)> f)
	{
		this->enableIfFn = f;
	}
	bool hasEnableIfFn() const
	{
		return static_cast<bool>(enableIfFn);
	}

	UIAnchor anchor;
protected:
	Vec2 size{ 0.0f, 0.0f };
	std::function<void(UIElement*)> fn_update{ {} };
	/*
	* the destroyIf function is called before the update function.
	*/
	std::function<bool(UIElement*)> destroyIfFn{ {} };
	std::function<bool(UIElement*)> enableIfFn{ {} };
private:
	bool bDestroyed{ false };
	bool bEnable{ true };
};