#pragma once
#include <string_view>

#include "../rendering/Renderer.hpp"
#include "../entity/EntityComponentStorage.hpp"
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

using UIEntityIndex = EntityHandleIndex;

using UIEntityHandle = EntityHandle;

template<typename T>
class UIContainer {
public:
	static_assert(std::is_base_of<UIElement, T>::value, "UIContainer can only hold UIElement derived classes/structs!");

	UIEntityIndex create(const T& element)
	{
		if (freeElements.empty()) {
			UIEntityIndex index = (UIEntityIndex)elements.size();
			elements.updateMaxEntNum(static_cast<size_t>(index + 1));
			elements.insert(index, element);
			return index;
		}
		else {
			UIEntityIndex index = freeElements.back();
			freeElements.pop_back();
			elements.insert(index, element);
			return index;
		}
	}

	UIEntityHandle createAsHandle(const T& element)
	{
		UIEntityIndex rindex;
		if (freeElements.empty()) {
			UIEntityIndex index = (UIEntityIndex)elements.size();
			elements.updateMaxEntNum(static_cast<size_t>(index + 1));
			elements.insert(index, element);
			rindex =  index;
		}
		else {
			UIEntityIndex index = freeElements.back();
			freeElements.pop_back();
			elements.insert(index, element);
			rindex = index;
		}
		if (rindex >= indexToVersion.size()) {
			indexToVersion.resize(rindex + 1, 0);
		}
		indexToVersion[rindex] += 1;
		return UIEntityHandle{ rindex, indexToVersion[rindex] };
	}

	void destroy(UIEntityIndex index)
	{
		if (elements.contains(index)) {
			elements.remove(index);
			freeElements.push_back(index);
		}
	}

	void destroy(UIEntityHandle entity)
	{
		assert(contains(entity));
		if (elements.contains(entity.index)) {
			elements.remove(entity.index);
			freeElements.push_back(entity.index);
		}
	}

	T& get(const UIEntityIndex index)
	{
		return elements.get(index);
	}
	const T& get(const UIEntityIndex index) const
	{
		return elements.get(index);
	}

	T& get(const UIEntityHandle entity)
	{
		assert(contains(entity));
		return elements.get(entity.index);
	}
	const T& get(const UIEntityHandle entity) const
	{
		assert(contains(entity));
		return elements.get(entity.index);
	}

	bool contains(UIEntityIndex index) const
	{
		return elements.contains(index);
	}

	bool contains(UIEntityHandle entity) const
	{
		const bool versionValid = isHandleValid(entity);
		return versionValid && elements.contains(entity.index);
	}

	EntityHandleVersion getVersion(UIEntityIndex index) const
	{
		return indexToVersion.at(index);
	}

	bool isHandleValid(UIEntityHandle entity) const {
		return entity.index < indexToVersion.size() && entity.version == indexToVersion[entity.index];
	}

	size_t size() const
	{
		return elements.size();
	}

	auto begin() { return elements.begin(); }
	auto end() { return elements.end(); }
private:
	ComponentStoragePagedIndexing<T> elements;
	std::vector<UIEntityIndex> freeElements;
	std::vector<EntityHandleVersion> indexToVersion;
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