#pragma once

#include "GUICommonElements.hpp"

namespace gui {
	template<typename T>
	u32* getChild(T&) { return nullptr; }

	template<> inline u32* getChild(Box& self) { return &self.child; }
	template<> inline u32* getChild(DropBox& self) { return &self.child; }
	template<> inline u32* getChild(DragDroppable& self) { return &self.child; }
	template<> inline u32* getChild(_ScrollBox& self) { return &self.child; }

	template<typename T>
	std::vector<u32>* getChildren(T&) { return nullptr; }

	template<> inline std::vector<u32>* getChildren(Group& self) { return &self.children; }

	template<typename T>
	std::array<u32,2>* getChildPair(T& d) { return nullptr; }
	
	template<> inline std::array<u32, 2>* getChildPair(HeadTail& self) { return &self.children; }
}
