#pragma once

#include "../GUIManager.hpp"

namespace gui {

	/**
	 * Specialize this template per type that should have different destroy functionality.
	 * 
	 * \param manager containing the elements.
	 * \param elementindex of the element to destroy.
	 * \param destroylist list one can append children to wich should be destroyed.
	 */
	template<typename T>
	void onDestroy(Manager& manager, T& elementindex, std::vector<u32>& destroylist) { }

	template<> void onDestroy<Column>(Manager& manager, Column& self, std::vector<u32>& destroylist)
	{
		for (u32 childid : self.children) {
			destroylist.push_back(childid);
		}
	}

	inline static void destroy(Manager& manager, ElementVariant& var, std::vector<u32>& destroylist)
	{
		std::visit([&](auto&& element) {onDestroy(manager, element, destroylist); }, var);
	}
}
