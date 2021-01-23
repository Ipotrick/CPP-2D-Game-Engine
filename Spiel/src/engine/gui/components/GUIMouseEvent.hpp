#pragma once

#include "../GUIManager.hpp"

namespace gui {

	template<typename T>
	void onMouseEvent(Manager& manager, T& self, u32 id, u32 rootid) {}

	template<> void onMouseEvent<_Button>(Manager& manager, _Button& self, u32 id, u32 rootid);
	template<> void onMouseEvent<DragField>(Manager& manager, DragField& self, u32 id, u32 rootid);

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

	template<> inline void onMouseEvent<DragField>(Manager& manager, DragField& self, u32 id, u32 rootid)
	{
		if (manager.window->buttonJustPressed(MouseButton::MB_LEFT)) {
			manager.draggedElement = { id, RootElementHandle{ rootid, manager.rootElements[rootid].version } };
		}
	}

	template<> inline void onMouseEvent(Manager& manager, _TextInput& self, u32 id, u32 rootid)
	{
		if (manager.window->buttonJustPressed(MouseButton::MB_LEFT)) {
			manager.focusedTextInput = { id, RootElementHandle{ rootid, manager.rootElements[rootid].version } };
		}
	}
}
