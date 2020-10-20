#pragma once
#include <string_view>

#include "Renderer.hpp"
#include "EntityComponentStorage.hpp"
#include "UIElement.hpp"
#include "UIFrame.hpp"
#include "UIText.hpp"
#include "UIButton.hpp"
#include "UIBar.hpp"
#include "UIList.hpp"
#include "UIField.hpp"
#include "UIMultiParent.hpp"
#include "UIClickable.hpp"
#include "UIPair.hpp"
#include "UISeperator.hpp"
#include "UICollapsable.hpp"

using UIEntity = Entity;

template<typename T>
class UIContainer {
public:
	static_assert(std::is_base_of<UIElement, T>::value);
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

	size_t size() const
	{
		return elements.size();
	}

	auto begin() { return elements.begin(); }
	auto end() { return elements.end(); }
private:
	ComponentStorage<T, paged_indexing> elements;
	std::vector<UIEntity> freeElements;
};

using UIElementTuple = std::tuple< 
	UIContainer<UIFrame>,
	UIContainer<UIText>, 
	UIContainer<UIButton>,
	UIContainer<UIBar>,
	UIContainer<UIPair>,
	UIContainer<UIList8>,
	UIContainer<UIList16>,
	UIContainer<UIList32>,
	UIContainer<UIList64>,
	UIContainer<UIField8>,
	UIContainer<UIField16>,
	UIContainer<UIField32>,
	UIContainer<UIField64>,
	UIContainer<UISeperator>,
	UIContainer<UICollapsable>
>;

template< size_t I, typename T, typename Tuple_t>
constexpr size_t index_in_ui_storagetuple_fn()
{
	static_assert(I < std::tuple_size<Tuple_t>::value, "the given ui element type is unknown");

	typedef typename std::tuple_element<I, Tuple_t>::type el;
	if constexpr (std::is_same<UIContainer<T>, el>::value) {
		return I;
	}
	else {
		return index_in_ui_storagetuple_fn<I + 1, T, Tuple_t>();
	}
}

template<typename T>
constexpr int getUIElementTypeIndex()
{
	return index_in_ui_storagetuple_fn<0, T, UIElementTuple>();
}