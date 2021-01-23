#pragma once

#include "../GUIManager.hpp"

namespace gui {

	template<typename T>
	void onDraw(Manager& manager, T& element, u32 id, DrawContext const& context, std::vector<Sprite>& out) {}

	template<> void onDraw(Manager& manager, Box& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);
	template<> void onDraw(Manager& manager, FillBox& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);
	template<> void onDraw(Manager& manager, Column& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);
	template<> void onDraw(Manager& manager, _Button& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);
	template<> void onDraw(Manager& manager, StaticText& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);
	template<> void onDraw(Manager& manager, _TextInput& self, u32 id, DrawContext const& context, std::vector<Sprite>& out);

	inline void draw(Manager& manager, ElementVariant& var, u32 id, DrawContext const& context, std::vector<Sprite>& out)
	{
		std::visit([&](auto&& element) { onDraw(manager, element, id, context, out); }, var);
	}

	/**
	 * \return the UNSCALED minimum size the element needs
	 */
	template<typename T>
	Vec2 minsizeOf(Manager& manager, T& element) { return Vec2{ 0,0 }; }
	template<> Vec2 minsizeOf(Manager& manager, Box& self);
	template<> Vec2 minsizeOf(Manager& manager, FillBox& self);
	template<> Vec2 minsizeOf(Manager& manager, Column& self);
	template<> Vec2 minsizeOf(Manager& manager, _Button& self);
	template<> Vec2 minsizeOf(Manager& manager, StaticText& self);
	template<> Vec2 minsizeOf(Manager& manager, _TextInput& self);

	inline Vec2 minsizeOf(Manager& manager, u32 id)
	{
		if (!manager.minsizes[id].has_value()) {
			std::visit([&](auto&& element) { manager.minsizes[id] = minsizeOf(manager, element); }, manager.elements[id]);
		}
		return manager.minsizes[id].value();
	}

	  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	 ////////////////////////////////////////////////// TYPE SPECIFIC IMPLEMENTATIONS: ////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	inline void drawRoot(Manager& manager, u32 id, std::vector<Sprite>& out)
	{
		Root& self = manager.rootElements[id].element;
		if (self.child != INVALID_ELEMENT_ID) {
			DrawContext myContext;
			myContext.scale = manager.globalScaling;
			myContext.topleft = Vec2{ 0, cast<f32>(manager.window->getHeight()) };
			myContext.bottomright = Vec2{ cast<f32>(manager.window->getWidth()), 0 };

			auto scaledSize = getSize(self.sizeing, myContext);
			auto place = getPlace(scaledSize, self.placeing, myContext);
			DrawContext childContext;
			childContext.scale *= manager.globalScaling;
			childContext.root = id;
			fit(childContext, scaledSize, place);
			draw(manager, manager.elements[self.child], self.child, childContext, out);
		}
	}

	template<> inline Vec2 minsizeOf<Box>(Manager& manager, Box& self)
	{
		return self.size;
	}
	template<> inline void onDraw<Box>(Manager& manager, Box& self, u32 id, DrawContext const& context, std::vector<Sprite>& out)
	{
		auto scaledSize = self.size * context.scale;
		auto place = getPlace(scaledSize, context);
		if (self.color.a != 0.0f) {
			out.push_back(
				Sprite{
					.color = self.color,
					.position = Vec3{place, context.renderDepth},
					.scale = scaledSize,
					.form = Form::Rectangle,
					.cornerRounding = 3.0f * context.scale,
					.drawMode = RenderSpace::PixelSpace
				}
			);
		}
		auto childcontext = context;
		childcontext.xalign = self.xalignment;
		childcontext.yalign = self.yalignment;
		fit(childcontext, scaledSize, place);
		fit(childcontext, self.padding);
		if (self.child != INVALID_ELEMENT_ID) {
			draw(manager, manager.elements[self.child], self.child, childcontext, out);
		}
	}

	template<> inline Vec2 minsizeOf<FillBox>(Manager& manager, FillBox& self)
	{
		if (self.child != INVALID_ELEMENT_ID)	return minsizeOf(manager, self.child);
		else									return Vec2{ 0,0 };
	}
	template<> inline void onDraw<FillBox>(Manager& manager, FillBox& self, u32 id, DrawContext const& context, std::vector<Sprite>& out)
	{
		auto scaledSize = context.size();
		auto place = context.centerpos();
		out.push_back(
			Sprite{
				.color = self.color,
				.position = Vec3{place, context.renderDepth},
				.scale = scaledSize,
				.form = Form::Rectangle,
				.cornerRounding = 3.0f * context.scale,
				.drawMode = RenderSpace::PixelSpace
			}
		);
		auto childcontext = context;
		childcontext.xalign = self.xalignment;
		childcontext.yalign = self.yalignment;
		fit(childcontext, self.padding);
		if (self.child != INVALID_ELEMENT_ID) {
			draw(manager, manager.elements[self.child], self.child, childcontext, out);
		}
	}

	template<> inline Vec2 minsizeOf<Column>(Manager& manager, Column& self)
	{
		Vec2 minsize = Vec2{ 
			cast<float>(self.padding.left + self.padding.right), 
			cast<float>(self.padding.top + self.padding.bottom + (self.children.size()-1) * self.spaceing)
		};
		for (u32 child : self.children) {
			auto childMinSize = minsizeOf(manager, child);
			minsize.x = std::max(minsize.x, childMinSize.x);
			minsize.y += childMinSize.y;
		}
		return minsize;
	}
	template<> inline void onDraw<Column>(Manager& manager, Column& self, u32 id, DrawContext const& context, std::vector<Sprite>& out)
	{
		const float scaledSpaceing = self.spaceing * context.scale;

		float childrenScaledY{ 0.0f };
		for (auto& child : self.children) {
			childrenScaledY += minsizeOf(manager, child).y;
		}
		childrenScaledY *= context.scale;
		const float childCount = static_cast<float>(self.children.size());
		float allSpacingY{ (childCount - 1) * scaledSpaceing };

		const float leftYSpace = context.size().y - scaledSpaceing * (childCount-1.0f) - childrenScaledY;
		const float uniformPerChildSpace = leftYSpace / childCount;

		DrawContext childContext = context;
		childContext.xalign = self.xalignment;
		childContext.yalign = self.yalignment;
		fit(childContext, self.padding);
		
		float offset{ 0.0f };
		if (self.packing == Packing::Tight) {
			if (self.yalignment == YAlign::Center)
				offset += (context.size().y - childrenScaledY - allSpacingY) * 0.5f;
			if (self.yalignment == YAlign::Bottom)
				offset += context.size().y - childrenScaledY - allSpacingY;
		}

		for (auto& childid : self.children) {
			ElementVariant& childvar = manager.elements[childid];
			const auto scaledSize = minsizeOf(manager, childid) * context.scale;

			DrawContext myContext = childContext;

			if (self.packing == Packing::Tight) {
				myContext.cutTop(offset);
				myContext.cutBottom(context.size().y - scaledSize.y - offset);
				offset += scaledSize.y + scaledSpaceing;
			}
			else {
				myContext.cutTop(offset);
				myContext.cutBottom(context.size().y - scaledSize.y - uniformPerChildSpace - offset);
				offset += scaledSize.y + scaledSpaceing + uniformPerChildSpace;
			}
			draw(manager, childvar, childid, myContext, out);
		}
	}

	template<> inline Vec2 minsizeOf<_Button>(Manager& manager, _Button& self)
	{
		return self.size;
	}
	template<> inline void onDraw<_Button>(Manager& manager, _Button& self, u32 id, DrawContext const& context, std::vector<Sprite>& out)
	{
		auto scaledSize = self.size * context.scale;
		auto place = getPlace(scaledSize, context);

		bool cursorOverElement{ false };
		Vec2 cursor = manager.renderer->convertCoordSys(manager.window->getCursorPos(), RenderSpace::WindowSpace, RenderSpace::PixelSpace);
		Vec2 relCursor = cursor - place;
		cursorOverElement =
			relCursor.x < scaledSize.x * 0.5f &&
			relCursor.y < scaledSize.y * 0.5f &&
			relCursor.x > -scaledSize.x * 0.5f &&
			relCursor.y > -scaledSize.y * 0.5f;

		if (cursorOverElement) {
			manager.requestMouseEvent(id, context.root, context.renderDepth);
		}
		else {
			self.bHold = false;
			self.bHover = false;
		}

		if (self.bHover) {		// bHover is not the same as cursorOverElement, bHover is only set by a successfull mouse event request
			scaledSize *= 1.1;
		}

		Vec4 color = self.bHold ? Vec4::From256(38, 241, 244) : Vec4::From256(10, 57, 58);

		out.push_back(
			Sprite{
				.color = color,
				.position = Vec3{place, context.renderDepth },
				.scale = scaledSize,
				.form = Form::Rectangle,
				.cornerRounding = 3.0f * context.scale,
				.drawMode = RenderSpace::PixelSpace
			}
		);
	}

	template<> inline Vec2 minsizeOf<DragField>(Manager& manager, DragField& self)
	{
		return Vec2{ 0,0 };
	}
	template<> inline void onDraw<DragField>(Manager& manager, DragField& self, u32 id, DrawContext const& context, std::vector<Sprite>& out)
	{
		auto scaledSize = context.size();
		auto place = context.centerpos();

		bool cursorOverElement{ false };
		Vec2 cursor = manager.renderer->convertCoordSys(manager.window->getCursorPos(), RenderSpace::WindowSpace, RenderSpace::PixelSpace);
		Vec2 relCursor = cursor - place;
		cursorOverElement =
			relCursor.x < scaledSize.x * 0.5f &&
			relCursor.y < scaledSize.y * 0.5f &&
			relCursor.x > -scaledSize.x * 0.5f &&
			relCursor.y > -scaledSize.y * 0.5f;
		
		if (cursorOverElement) {
			manager.requestMouseEvent(id, context.root, context.renderDepth);
		}

		out.push_back(
			Sprite{
				.color = self.color,
				.position = Vec3{place, context.renderDepth},
				.scale = scaledSize,
				.form = Form::Rectangle,
				.cornerRounding = 3.0f * context.scale,
				.drawMode = RenderSpace::PixelSpace
			}
		);
	}
}
