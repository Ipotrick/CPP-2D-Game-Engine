#pragma once

#include <any>

#include "../base/GUIElement.hpp"
#include "../components/GUITextElements.hpp"

namespace gui {

	struct Group;

	struct Column : IElement {
		std::function<void(Group&, u32)> onUpdate;
		ValueOrPtr<XAlign> xalign{ XAlign::Left };
		ValueOrPtr<YAlign> yalign{ YAlign::Top };
		ValueOrPtr<Padding> padding{ Padding{NAN,NAN,NAN,NAN} };
		ValueOrPtr<Packing> packing{ Packing::Tight };
		ValueOrPtr<f32> spacing{ NAN };
		std::vector<u32> children;
	};

	struct Row : IElement {
		std::function<void(Group&, u32)> onUpdate;
		ValueOrPtr<XAlign>xalign{ XAlign::Left };
		ValueOrPtr<YAlign>yalign{ YAlign::Center };
		ValueOrPtr<Padding>padding{ Padding{NAN,NAN,NAN,NAN} };
		ValueOrPtr<Packing>packing{ Packing::Tight };
		ValueOrPtr<f32>spacing{ NAN };
		std::vector<u32> children;
	};

	struct Group : IElement {
		Group(Column&& r) :
			onUpdate{ std::move(r.onUpdate) },
			xalign{ std::move(r.xalign) },
			yalign{ std::move(r.yalign) },
			padding{ std::move(r.padding) },
			packing{ std::move(r.packing) },
			spacing{ std::move(r.spacing) },
			bVertical{ true },
			children{ std::move(r.children) }
		{}
		Group(Row&& r) : 
			onUpdate{ std::move(r.onUpdate) },
			xalign{ std::move(r.xalign) },
			yalign{ std::move(r.yalign) },
			padding{ std::move(r.padding) },
			packing{ std::move(r.packing) },
			spacing{ std::move(r.spacing) },
			bVertical{false},
			children{std::move(r.children)}
		{}

		std::function<void(Group&, u32)> onUpdate;
		ValueOrPtr<XAlign> xalign{ XAlign::Left };
		ValueOrPtr<YAlign> yalign{ YAlign::Top };
		ValueOrPtr<Padding> padding{ Padding{NAN,NAN,NAN,NAN} };
		ValueOrPtr<Packing> packing{ Packing::Tight };
		ValueOrPtr<f32> spacing{ NAN };
		ValueOrPtr<bool> bVertical{ true };
		std::vector<u32> children;
	};


	enum class OnDrag { Move, Resize };
	struct Box : IElement {
		std::function<void(Box&, u32)> onUpdate;
		ValueOrPtr<Vec2> minsize{ Vec2{} };
		ValueOrPtr<bool> bFillSpace{ false };
		ValueOrPtr<Vec4> color{ UNSET_COLOR };
		ValueOrPtr<XAlign> xalign{ XAlign::Left };
		ValueOrPtr<YAlign> yalign{ YAlign::Top };
		ValueOrPtr<Padding> padding{ Padding{NAN, NAN, NAN, NAN} };
		ValueOrPtr<bool> bDragable{ false };
		ValueOrPtr<OnDrag> onDrag{ OnDrag::Move };
		u32 child{ INVALID_ELEMENT_ID };
	};



	struct Button : IElement {
		std::function<void(Button&, u32)> onUpdate;
		Vec2 size{ Vec2{15,15} };
		Vec4 color{ UNSET_COLOR };
		Vec4 holdColor{ UNSET_COLOR };
		std::function<void(Button& self)> onPress;
		std::function<void(Button& self)> onHold;
		std::function<void(Button& self)> onRelease;
		u32 child{ INVALID_ELEMENT_ID };
	};
	namespace {
		struct _Button : public Button {
			_Button(Button&& b) : Button{ b } {}
			bool bHold{ false };
			bool bHover{ false };
		};
	}



	struct Checkbox : IElement {
		std::function<void(Checkbox&, u32)> onUpdate;
		bool* value{ nullptr };
		ValueOrPtr<Vec2> size{ Vec2{15,15} };
		ValueOrPtr<Vec4> color{ UNSET_COLOR };
		ValueOrPtr<Vec4> colorEnabled{ UNSET_COLOR };
		ValueOrPtr<Vec4> colorDisabled{ UNSET_COLOR };
	};
	namespace {
		struct _Checkbox : public Checkbox {
			_Checkbox(Checkbox&& e) : Checkbox{e} {}
			bool bHold{ false };
			bool bHover{ false };
		};
	}



	struct SliderF64 : IElement {
		std::function<void(SliderF64&, u32)> onUpdate;
		f64* value{ nullptr };
		ValueOrPtr<Vec2> size{ Vec2{150,25} };
		ValueOrPtr<f64> min{ 0.0 };
		ValueOrPtr<f64> max{ 1.0 };
		bool bVertical{ false };
		bool bThin{ true };
		ValueOrPtr<Vec4> colorBar{ UNSET_COLOR };
		ValueOrPtr<Vec4> colorSlider{ UNSET_COLOR };
		ValueOrPtr<Vec4> colorError{ UNSET_COLOR };
		u32 child{ INVALID_ELEMENT_ID };
	};



	struct DragDroppable : IElement {
		std::function<void(DragDroppable&, u32)> onUpdate;
		std::any data;
		ValueOrPtr<bool> bCatchable{ true };
		u32 child{ INVALID_ELEMENT_ID };
	};



	struct DropBox : IElement {
		std::function<void(DropBox&, u32)> onUpdate;
		// gets executed in a catch event
		// the u32 represents the id of the catched element
		// return true when we want to catch the element and make it our child
		std::function<bool(DropBox&, DragDroppable&)> onCatch;
		std::any data;
		ValueOrPtr<Vec2> minsize{ Vec2{ 50,50 } };
		ValueOrPtr<Vec4> color{ UNSET_COLOR };
		// true: mouse input will be catched by the element.
		// false: mouse input will fall throu to the element below.*/
		bool bCatchMouseInput{ true };
		u32 child{ INVALID_ELEMENT_ID };
	};



	struct Footer : IElement {
		std::function<void(Footer&, u32)> onUpdate;
		ValueOrPtr<Mode> mode{ Mode::Absolute };
		ValueOrPtr<f32> size{ 30 };
		ValueOrPtr<f32> spacing{ NAN };
		std::vector<u32> children;
	};
}
