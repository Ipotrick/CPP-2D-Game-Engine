#pragma once
#include <string_view>

#include "Renderer.hpp"
#include "EntityComponentStorage.hpp"
#include "UIElement.hpp"
#include "UIFrame.hpp"
#include "UIText.hpp"
#include "UIButton.hpp"

using UIEntity = Entity;

template<typename T>
class UIContainer {
public:
	UIEntity create(const T& element)
	{
		if (freeElements.empty()) {
			Entity index = (Entity)elements.size();
			elements.updateMaxEntNum(index + 1);
			elements.insert(index, element);
			return index;
		}
		else {
			Entity index = freeElements.back();
			freeElements.pop_back();
			elements.insert(index, element);
			return index;
		}
	}
	UIEntity create(T&& element)
	{
		if (freeElements.empty()) {
			Entity index = (Entity)elements.size();
			elements.updateMaxEntNum(index + 1);
			elements.insert(index, element);
			return index;
		}
		else {
			Entity index = freeElements.back();
			freeElements.pop_back();
			elements.insert(index, element);
			return index;
		}
	}
	void destroy(UIEntity index)
	{
		if (elements.contains(index)) {
			elements.remove(index);
			freeElements.push_back(index);
		}
	}

	T& get(const UIEntity index)
	{
		return elements.get(index);
	}
	const T& get(const UIEntity index) const
	{
		return elements.get(index);
	}

	bool contains(UIEntity index) const
	{
		return elements.contains(index);
	}

	auto begin() { return elements.begin(); }
	auto end() { return elements.end(); }
private:
	ComponentStorage<T, paged_indexing> elements;
	std::vector<UIEntity> freeElements;
};

using UIElementTuple = std::tuple< 
	UIContainer<UIText>, 
	UIContainer<UIButton>
>;

template<typename T>
constexpr int getUIElementTypeIndex()
{
	static_assert(false, "given type is not an ui element!, You can register a component by adding it into the tuple above and then overriding this template as seen below!");
}

template<>
constexpr int getUIElementTypeIndex<UIText>()
{
	return 0;
}

template<>
constexpr int getUIElementTypeIndex<UIButton>()
{
	return 1;
}
