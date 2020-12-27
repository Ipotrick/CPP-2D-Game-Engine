#pragma once

#include <functional>
#include <optional>

#include "UIContext.hpp"
#include "UIAnchor.hpp"

static constexpr Vec2 standartBorder{ 1.0f, 1.0f };

class UIElementPrivilage {};

class UIElement {
public:
	virtual void draw(std::vector<Sprite>& buffer, UIContext context) = 0;

	struct Parameters {
		// UIElement:
		UIAnchor anchor;
		Vec2 size{ 0.0f, 0.0f };
		std::function<void(UIElement*)> fn_update{ {} };
		std::function<bool(UIElement*)> fn_enableIf{ {} };
	};

	UIElement(Parameters param = Parameters()):
		anchor{param.anchor},
		size{ param.size },
		fn_update{ param.fn_update},
		fn_enableIf{ param.fn_enableIf }
	{ }

	void update()
	{
		if (!isDestroyed()) {
			if (hasEnableIfFn()) {
				if (fn_enableIf(this)) {
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
	* enables element when function returns true
	*/
	void setEnableIfFn(std::function<bool(UIElement*)> f)
	{
		this->fn_enableIf = f;
	}
	bool hasEnableIfFn() const
	{
		return static_cast<bool>(fn_enableIf);
	}

	UIAnchor anchor;
protected:
	friend class UISingleParent;
	friend class UIManager;
	friend class UIPair;
	template<std::size_t SIZE>
	friend class UIMultiParent;
	virtual void destroy()
	{
		bDestroyed = true;
	}
	bool isDestroyed() const { return bDestroyed; }

	Vec2 size{ 0.0f, 0.0f };
	std::function<void(UIElement*)> fn_update{ {} };
	std::function<bool(UIElement*)> fn_enableIf{ {} };
private:
	bool bDestroyed{ false };
	bool bEnable{ true };
};