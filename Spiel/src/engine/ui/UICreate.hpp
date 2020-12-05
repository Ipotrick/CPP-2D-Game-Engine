#pragma once

#include <initializer_list>

#include "../EngineCore.hpp"

namespace ui {
	template<class T>
	UIElement* makeui(T uient)
	{
		static_assert(std::is_base_of_v<UIElement, T>);

		return EngineCore::ui.createAndGet(uient);
	}

	template<class T>
	UIElement* makeui(T uient, std::initializer_list<UIElement*> children)
	{
		static_assert(std::is_base_of_v<UIElement, T>);

		for (auto child : children) {
			uient.addChild(child);
		}

		return EngineCore::ui.createAndGet(uient);
	}

	template<>
	inline UIElement* makeui<UIPair>(UIPair uient, std::initializer_list<UIElement*> children)
	{
		assert(children.size() == 2);
		uient.setFirst(*(children.begin() + 0));
		uient.setSecond(*(children.begin() + 1));

		return EngineCore::ui.createAndGet(uient);
	}

	UIElement* pair(UIPair::Parameters param, std::initializer_list<UIElement*> children);
	UIElement* frame(UIFrame::Parameters param, std::initializer_list<UIElement*> children);
	UIElement* bar(UIBar::Parameters param);
	UIElement* text(UIText::Parameters param);
}