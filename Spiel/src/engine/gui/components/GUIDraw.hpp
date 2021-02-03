#pragma once


#include "../GUIManager.hpp"
#include "../base/GUITextUtil.hpp"

namespace gui {
	template<typename T>
	void onDraw(Manager& manager, T& element, u32 id, DrawContext const& context, std::vector<Sprite>& out) {}

	template<> void onDraw(Manager& manager, Box& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);
	template<> void onDraw(Manager& manager, Group& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);
	template<> void onDraw(Manager& manager, _Button& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);
	template<> void onDraw(Manager& manager, StaticText& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);
	template<> void onDraw(Manager& manager, Text& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);
	template<> void onDraw(Manager& manager, _TextInput& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);
	template<> void onDraw(Manager& manager, _Checkbox& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);
	template<> void onDraw(Manager& manager, SliderF64& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);
	template<> void onDraw(Manager& manager, DragDroppable& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);
	template<> void onDraw(Manager& manager, DropBox& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);
	template<> void onDraw(Manager& manager, _TextInputF64& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);
	template<> void onDraw(Manager& manager, Footer& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);

	inline void draw(Manager& manager, ElementVariant& var, u32 id, DrawContext const& context, std::vector<Sprite>& out)
	{
		std::visit([&](auto&& element) { onDraw(manager, element, id, context, out); }, var);
	}

	template<typename T>
	/**
	 * shoud allways be called on child first and then one should update oneself.
	 * \return UNSCALED minimum size the element needs.
	 */
	Vec2 updateAndGetMinsize(Manager& manager, T& element) { static_assert(false); return {}; }
	template<> Vec2 updateAndGetMinsize(Manager& manager, std::monostate& self) { return {}; }
	template<> Vec2 updateAndGetMinsize(Manager& manager, Box& self);
	template<> Vec2 updateAndGetMinsize(Manager& manager, Group& self);
	template<> Vec2 updateAndGetMinsize(Manager& manager, _Button& self);
	template<> Vec2 updateAndGetMinsize(Manager& manager, StaticText& self);
	template<> Vec2 updateAndGetMinsize(Manager& manager, Text& self);
	template<> Vec2 updateAndGetMinsize(Manager& manager, _TextInput& self);
	template<> Vec2 updateAndGetMinsize(Manager& manager, _Checkbox& self);
	template<> Vec2 updateAndGetMinsize(Manager& manager, SliderF64& self);
	template<> Vec2 updateAndGetMinsize(Manager& manager, DragDroppable& self);
	template<> Vec2 updateAndGetMinsize(Manager& manager, DropBox& self);
	template<> Vec2 updateAndGetMinsize(Manager& manager, _TextInputF64& self);
	template<> Vec2 updateAndGetMinsize(Manager& manager, Footer& self);

	/**
	 * shoud allways be called on child first and then one should update oneself.
	 * \return UNSCALED minimum size the element needs.
	 */
	inline Vec2 updateAndGetMinsize(Manager& manager, u32 id)
	{
		std::visit([&](auto&& element) { manager.minsizes[id] = updateAndGetMinsize(manager, element); }, manager.elements[id]);
		return manager.minsizes[id];
	}

	inline void drawRoot(Manager& manager, u32 id, std::vector<Sprite>& out)
	{
		Root& self = manager.rootElements[id].element;

		if (self.onUpdate) self.onUpdate(self);

		if (self.child != INVALID_ELEMENT_ID) {
			DrawContext myContext;
			myContext.scale = manager.globalScaling;
			myContext.topleft = Vec2{ 0, cast<f32>(manager.window->getHeight()) };
			myContext.bottomright = Vec2{ cast<f32>(manager.window->getWidth()), 0 };

			auto scaledSize = getSize(self.sizing, myContext);
			auto place = getPlace(scaledSize, self.placing, myContext);
			DrawContext childContext;
			childContext.scale *= manager.globalScaling;
			childContext.root = id;
			fit(childContext, scaledSize, place);
			updateAndGetMinsize(manager, self.child);
			draw(manager, manager.elements[self.child], self.child, childContext, out);
		}
	}


}
