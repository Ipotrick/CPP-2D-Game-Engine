#pragma once

#include "GUICommonElements.hpp"

namespace gui {
	template<typename T, typename ... Args>
	void onUpdate(Manager& manager, T& element, const Args& ... args) { static_assert(false, "ERROR: this template must be spezialised!"); }

	template<> void onUpdate<GUIButton, bool, Vec2, Vec2>(Manager& manager, GUIButton& self, const bool& cursorFocus, const Vec2& place, const Vec2& scaledSize)
	{
		bool hovered{ false };
		if (cursorFocus || true /*TODO REMOVE THE TRUE*/) {
			Vec2 cursor = manager.renderer->convertCoordSys(manager.in->getMousePosition(), RenderSpace::WindowSpace, RenderSpace::PixelSpace);
			Vec2 relCursor = cursor - place;
			hovered =
				relCursor.x < scaledSize.x * 0.5f &&
				relCursor.y < scaledSize.y * 0.5f &&
				relCursor.x > -scaledSize.x * 0.5f &&
				relCursor.y > -scaledSize.y * 0.5f;
		}

		if (hovered) {
			if (self._bHold && manager.in->buttonJustReleased(Button::MB_LEFT)) {
				if (self.onRelease) self.onRelease(self);
			}
			else if (self._bHold && manager.in->buttonPressed(Button::MB_LEFT)) {
				if (self.onHold) self.onHold(self);
			}
			else if (manager.in->buttonJustPressed(Button::MB_LEFT)) {
				if (self.onPress) self.onPress(self);
				self._bHold = true;
			}
			else if (self._bHold && !manager.in->buttonPressed(Button::MB_LEFT)) {
				self._bHold = false;
			}
		}
		else if (self._bHold) {
			self._bHold = false;
		}
	}
}
