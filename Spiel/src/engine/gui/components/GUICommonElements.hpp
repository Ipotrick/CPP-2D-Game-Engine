#pragma once

#include "../base/GUIElement.hpp"

namespace gui {
	struct Column : IElement {
		Vec2 minsize{ 10.0f, 10.0f };
		XAlign xalignment{ XAlign::Left };
		YAlign yalignment{ YAlign::Top };
		Listing listing{ Listing::Packed };
		float spaceing{ 0.0f };
		std::vector<u32> children;
	};

	struct Box : IElement {
		Vec2 minsize{ 10,10 };
		Vec4 color{ 1, 0.33, 0.66, 1 };
	};

	struct FillBox : IElement {
		Vec4 color{
			rand() % 1000 / 1000.0f,
			rand() % 1000 / 1000.0f,
			rand() % 1000 / 1000.0f,
			1.0f
		};
	};

	struct GUIButton : IElement {
		Vec2 size{ 15,15 };
		std::function<void(GUIButton& self)> onPress;
		std::function<void(GUIButton& self)> onHold;
		std::function<void(GUIButton& self)> onRelease;

		// Non initialization fields:
		bool _bHold{ false };
	};
}
