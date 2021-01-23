#pragma once

#include "../base/GUIElement.hpp"
#include "../components/GUITextElements.hpp"

namespace gui {
	struct Column : IElement {
		XAlign xalignment{ XAlign::Left };
		YAlign yalignment{ YAlign::Top };
		Padding padding{0,0,0,0};
		Packing packing{ Packing::Tight };
		float spaceing{ 5.0f };
		std::vector<u32> children;
	};

	struct Box : IElement {
		Vec2 size{ 10,10 };
		Vec4 color{ 1, 0.33, 0.66, 1 };
		XAlign xalignment{ XAlign::Left };
		YAlign yalignment{ YAlign::Top };
		Padding padding{0,0,0,0};
		u32 child{ INVALID_ELEMENT_ID };
	};

	struct FillBox : IElement {
		Vec4 color{0,0,0,1};
		XAlign xalignment{ XAlign::Left };
		YAlign yalignment{ YAlign::Top };
		Padding padding;
		u32 child{ INVALID_ELEMENT_ID };
	};

	struct Button : IElement {
		Vec2 size{ 15,15 };
		std::function<void(Button& self)> onPress;
		std::function<void(Button& self)> onHold;
		std::function<void(Button& self)> onRelease;
	};
	namespace {
		struct _Button : public Button {
			_Button(Button&& b) : Button{ b } {}
			bool bHold{ false };
			bool bHover{ false };
		};
	}

	struct DragField : IElement {
		Vec4 color = { 1,0.7,0.4, 1 };
	};
}
