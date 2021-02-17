#pragma once

#include "../GUIManager.hpp"

namespace gui {

	template<typename T>
	void onMouseEvent(Manager& manager, T& self, u32 id, u32 rootid) {}

	template<> void onMouseEvent(Manager& manager, _Button& self, u32 id, u32 rootid);
	template<> void onMouseEvent(Manager& manager, Box& self, u32 id, u32 rootid);
	template<> void onMouseEvent(Manager& manager, _Checkbox& self, u32 id, u32 rootid);
	template<> void onMouseEvent(Manager& manager, SliderF64& self, u32 id, u32 rootid);
	template<> void onMouseEvent(Manager& manager, DragDroppable& self, u32 id, u32 rootid);

	inline void onMouseEvent(Manager& manager, ElementVariant& var, u32 id, u32 rootid)
	{
		std::visit([&](auto&& element) { onMouseEvent(manager, element, id, rootid); }, var);
	}

	  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	 ////////////////////////////////////////////////// TYPE SPECIFIC IMPLEMENTATIONS: ////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<> inline void onMouseEvent<_Button>(Manager& manager, _Button& self, u32 id, u32 rootid)
	{
		self.bHover = true;

		if (self.bHold && manager.window->buttonJustReleased(MouseButton::MB_LEFT)) {
			if (self.onRelease) self.onRelease(self);
		}
		else if (self.bHold && manager.window->buttonPressed(MouseButton::MB_LEFT)) {
			if (self.onHold) self.onHold(self);
		}
		else if (manager.window->buttonJustPressed(MouseButton::MB_LEFT)) {
			if (self.onPress) self.onPress(self);
			self.bHold = true;
		}
		else if (self.bHold && !manager.window->buttonPressed(MouseButton::MB_LEFT)) {
			self.bHold = false;
		}
	}

	template<> inline void onMouseEvent(Manager& manager, _TextInput& self, u32 id, u32 rootid)
	{
		if (manager.window->buttonJustPressed(MouseButton::MB_LEFT)) {
			manager.focusedTextInput = { id, Manager::RootHandle{ rootid, manager.rootElements[rootid].version } };
		}
	}

	template<> inline void onMouseEvent(Manager& manager, _TextInputF64& self, u32 id, u32 rootid)
	{
		if (manager.window->buttonJustPressed(MouseButton::MB_LEFT)) {
			manager.focusedTextInput = { id, Manager::RootHandle{ rootid, manager.rootElements[rootid].version } };
			self.str = "";
		}
	}

	template<> inline void onMouseEvent<Box>(Manager& manager, Box& self, u32 id, u32 rootid)
	{
		if (manager.window->buttonJustPressed(MouseButton::MB_LEFT)) {
			manager.draggedElement = { id, Manager::RootHandle{ rootid, manager.rootElements[rootid].version } };
		}
	}

	template<> inline void onMouseEvent<_Checkbox>(Manager& manager, _Checkbox& self, u32 id, u32 rootid)
	{
		self.bHover = true;
		if (self.bHold && manager.window->buttonJustReleased(MouseButton::MB_LEFT)) {
			if (self.value) *self.value = !*self.value;
		}
		else if (manager.window->buttonJustPressed(MouseButton::MB_LEFT)) {
			self.bHold = true;
		}
		else if (self.bHold && !manager.window->buttonPressed(MouseButton::MB_LEFT)) {
			self.bHold = false;
		}
	}

	template<> inline void onMouseEvent<SliderF64>(Manager& manager, SliderF64& self, u32 id, u32 rootid)
	{
		if (manager.window->buttonJustPressed(MouseButton::MB_LEFT) && self.max > self.min) {
			manager.draggedElement = { id, Manager::RootHandle{ rootid, manager.rootElements[rootid].version } };
		}
	}

	template<> inline void onMouseEvent(Manager& manager, DragDroppable& self, u32 id, u32 rootid)
	{
		if (manager.window->buttonJustPressed(MouseButton::MB_LEFT)) {
			manager.draggedElement = { id, Manager::RootHandle{ rootid, manager.rootElements[rootid].version } };
		}
	}

	template<> inline void onMouseEvent(Manager& manager, DropBox& self, u32 id, u32 rootid)
	{
		if (self.child == INVALID_ELEMENT_ID) {
			if (manager.window->buttonJustReleased(MouseButton::MB_LEFT) && manager.droppedElement && manager.droppedElement->bCatchable) {
				if (!self.onCatch || self.onCatch(self, *manager.droppedElement)) {
					manager.changeParent(manager.droppedElementId, id);
				}
			}
		}
	}
}
