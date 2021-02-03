#pragma once

#include "GUIDraw.hpp"

namespace gui {

	template<> inline Vec2 updateAndGetMinsize<Box>(Manager& manager, Box& self)
	{
		Vec2 minsize;
		if (self.child != INVALID_ELEMENT_ID) {
			minsize = updateAndGetMinsize(manager, self.child);
		}
		if (self.onUpdate) self.onUpdate(self);
		minsize += size(self.padding);
		minsize = max(minsize, self.minsize);
		return minsize;
	}
	template<> inline void onDraw<Box>(Manager& manager, Box& self, u32 id, DrawContext const& context, std::vector<Sprite>& out)
	{
		const Vec2 scaledSize = (self.bFillSpace ? context.size() : manager.minsizes[id] * context.scale);
		const Vec2 place = getPlace(scaledSize, context);
		if (cast<Vec4&>(self.color).a != 0.0f) {
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

		if (self.bDragable) {
			const Vec2 cursor = manager.renderer->convertCoordSys(manager.window->getCursorPos(), RenderSpace::WindowSpace, RenderSpace::PixelSpace);
			const bool cursorOverElement = isCoordInBounds(scaledSize, place, cursor);

			if (cursorOverElement) {
				manager.requestMouseEvent(id, context.root, context.renderDepth);
			}
		}

		if (self.child != INVALID_ELEMENT_ID) {
			auto childcontext = context;
			childcontext.xalign = self.xalign;
			childcontext.yalign = self.yalign;
			fit(childcontext, scaledSize, place);
			fit(childcontext, self.padding);
			draw(manager, manager.elements[self.child], self.child, childcontext, out);
		}
	}
	
	template<> inline Vec2 updateAndGetMinsize<_Button>(Manager& manager, _Button& self)
	{
		if (self.onUpdate) self.onUpdate(self);
		return self.size;
	}
	template<> inline void onDraw<_Button>(Manager& manager, _Button& self, u32 id, DrawContext const& context, std::vector<Sprite>& out)
	{
		auto scaledSize = self.size * context.scale;
		auto place = getPlace(scaledSize, context);

		Vec2 cursor = manager.renderer->convertCoordSys(manager.window->getCursorPos(), RenderSpace::WindowSpace, RenderSpace::PixelSpace);
		const bool cursorOverElement = isCoordInBounds(scaledSize, place, cursor);

		if (cursorOverElement) {
			manager.requestMouseEvent(id, context.root, context.renderDepth);
		}
		else {
			self.bHold = false;
			self.bHover = false;
		}

		if (self.bHover) {		// bHover is not the same as cursorOverElement, bHover is only set by a successfull mouse event request
			scaledSize *= 0.95f;
		}

		out.push_back(
			Sprite{
				.color = self.bHold ? self.holdColor : self.color,
				.position = Vec3{place, context.renderDepth },
				.scale = scaledSize,
				.form = Form::Rectangle,
				.cornerRounding = 3.0f * context.scale,
				.drawMode = RenderSpace::PixelSpace
			}
		);

		if (self.text.has_value()) {
			if (!self.text.value().fonttexture.valid()) {
				manager.renderer->validateTextureRef(self.text.value().fonttexture);
			}

			auto t = self.text.value();
			auto c = context;
			fit(c, scaledSize, place);
			TextContext tcontext{ c };
			tcontext.scale *= (self.bHover ? 0.95f : 1.0f);
			tcontext.fontSize = t.fontSize;
			tcontext.texref = t.fonttexture.makeSmall();;
			tcontext.xalign = XAlign::Center;
			tcontext.yalign = YAlign::Center;
			drawText(t.value.data(), tcontext, out);
		}
	}

	template<> inline Vec2 updateAndGetMinsize(Manager& manager, _Checkbox& self)
	{
		if (self.onUpdate) self.onUpdate(self);
		return self.size;
	}
	template<> inline void onDraw<_Checkbox>(Manager& manager, _Checkbox& self, u32 id, DrawContext const& context, std::vector<Sprite>& out)
	{
		Vec2 scaledSize = self.size * context.scale;
		Vec2 place = getPlace(scaledSize, context);

		const Vec2 cursor = manager.renderer->convertCoordSys(manager.window->getCursorPos(), RenderSpace::WindowSpace, RenderSpace::PixelSpace);
		const bool cursorOverElement = isCoordInBounds(scaledSize, place, cursor);

		f32 HOVER_SCALING{ 0.9f };

		if (self.bHover) {
			scaledSize *= HOVER_SCALING;
		}

		out.push_back(
			Sprite{
				.color = self.color,
				.position = Vec3{place, context.renderDepth},
				.scale = scaledSize,
				.form = Form::Rectangle,
				.cornerRounding = 2.0f * context.scale,
				.drawMode = RenderSpace::PixelSpace
			}
		);

		if (self.value) {
			if (cursorOverElement) {
				manager.requestMouseEvent(id, context.root, context.renderDepth);
			}
			else {
				self.bHold = false;
				self.bHover = false;
			}

			f32 smallerDiemnsion = std::min(scaledSize.x, scaledSize.y);

			smallerDiemnsion *= 0.3f;
			scaledSize -= Vec2{ smallerDiemnsion ,smallerDiemnsion };

			out.push_back(
				Sprite{
					.color = (*self.value ? self.colorEnabled : self.colorDisabled),
					.position = Vec3{place, context.renderDepth },
					.scale = scaledSize,
					.form = Form::Rectangle,
					.cornerRounding = 1.5f * context.scale,
					.drawMode = RenderSpace::PixelSpace
				}
			);
		}
	}

	template<> inline Vec2 updateAndGetMinsize<SliderF64>(Manager& manager, SliderF64& self)
	{
		if (self.onUpdate) self.onUpdate(self);
		return self.size;
	}
	template<> inline void onDraw<SliderF64>(Manager& manager, SliderF64& self, u32 id, DrawContext const& context, std::vector<Sprite>& out)
	{
		const Vec2 responsiveArea = self.size * context.scale;
		const Vec2 place = getPlace(responsiveArea, context);
		const Vec2 scaledBarSize = responsiveArea * (self.bVertical ? Vec2{ 0.2f, 0.95f } : Vec2{ 0.95f, 0.2f });
		const f32 barThiccness = std::min(scaledBarSize.x, scaledBarSize.y);
		const f32 sliderBlobSize = std::min(responsiveArea.x, responsiveArea.y) * 0.8f;
		const bool rangeOK{ self.max > self.min };
		// Draw bar:
		out.push_back(
			Sprite{
				.color = rangeOK ? self.colorBar : self.colorError,
				.position = Vec3{place, context.renderDepth},
				.scale = scaledBarSize,
				.form = Form::Rectangle,
				.cornerRounding = barThiccness * 0.5f,
				.drawMode = RenderSpace::PixelSpace
			}
		);

		const Vec2 cursor = manager.renderer->convertCoordSys(manager.window->getCursorPos(), RenderSpace::WindowSpace, RenderSpace::PixelSpace);
		if (isCoordInBounds(responsiveArea, place, cursor)) {
			manager.requestMouseEvent(id, context.root, context.renderDepth);
		}

		auto drawInvalidSlider = [&] () {  
			out.push_back(
				Sprite{
					.color = rangeOK ? self.colorBar : self.colorError,
					.position = Vec3{place, context.renderDepth},
					.scale = Vec2{sliderBlobSize,sliderBlobSize},
					.form = Form::Rectangle,
					.cornerRounding = sliderBlobSize * 0.5f,
					.drawMode = RenderSpace::PixelSpace
				}
			);
		};

		if (self.value) {
			const f32 sliderMinPosMainAxis	= (self.bVertical ? place.y - scaledBarSize.y * 0.5f : place.x - scaledBarSize.x * 0.5f) + sliderBlobSize * 0.5f;
			const f32 sliderMaxPosMainAxis	= (self.bVertical ? place.y + scaledBarSize.y * 0.5f : place.x + scaledBarSize.x * 0.5f) - sliderBlobSize * 0.5f;
			const f32 clampedCursorPos		= std::clamp((self.bVertical ? cursor.y : cursor.x), sliderMinPosMainAxis, sliderMaxPosMainAxis);
			const f32 relativeCursorPos		= (clampedCursorPos - sliderMinPosMainAxis) / (sliderMaxPosMainAxis - sliderMinPosMainAxis);;

			const bool bDragged = manager.draggedElement.first == id;
			if (bDragged) { 
				*self.value = self.min + (self.max - self.min) * relativeCursorPos;
			}
			if (*self.value > self.max || *self.value < self.min /* does the value escape the min-max-range */) { 
				drawInvalidSlider(); 
			}
			else {
				const Vec2 sliderMinPos = (self.bVertical ? Vec2{place.x, sliderMinPosMainAxis } : Vec2{ sliderMinPosMainAxis, place.y});
				const Vec2 sliderMaxPos = (self.bVertical ? Vec2{place.x, sliderMaxPosMainAxis } : Vec2{ sliderMaxPosMainAxis, place.y});
				const f32 relativeSliderValue = (*self.value - self.min) / (self.max - self.min);
				const Vec2 sliderPos = sliderMinPos + (sliderMaxPos - sliderMinPos) * relativeSliderValue;

				// Draw Slider indexer:
				out.push_back(
					Sprite{ 
						.color = self.colorSlider * (bDragged ? 0.9f : 1.0f),
						.position = Vec3{sliderPos, context.renderDepth},
						.scale = Vec2{sliderBlobSize,sliderBlobSize},
						.form = Form::Rectangle,
						.cornerRounding = sliderBlobSize * 0.5f,
						.drawMode = RenderSpace::PixelSpace
					}
				);
			}
		}
		else {
			drawInvalidSlider();
		}
	}

	template<> inline Vec2 updateAndGetMinsize(Manager& manager, Group& self)
	{
		Vec2 minsize;
		for (u32 child : self.children) {
			auto childMinSize = updateAndGetMinsize(manager, child);
			if (self.bVertical) {
				minsize.x = std::max(minsize.x, childMinSize.x);
				minsize.y += childMinSize.y;
			}
			else {
				minsize.y = std::max(minsize.y, childMinSize.y);
				minsize.x += childMinSize.x;
			}
		}

		if (self.bVertical) {
			minsize.y += (self.children.size() - 1) * self.spacing;
		}
		else {
			minsize.x += (self.children.size() - 1) * self.spacing;
		}
		if (self.onUpdate) { self.onUpdate(self); }
		minsize += size(self.padding);
		return minsize;
	}
	template<> inline void onDraw<Group>(Manager& manager, Group& self, u32 id, DrawContext const& context, std::vector<Sprite>& out)
	{
		const Vec2 scaledWholeSize = context.size();
		const Vec2 scaledHalfSize = scaledWholeSize  * 0.5f;
		const Vec2 place = getPlace(scaledWholeSize, context);
		const f32 scaledSpaceing = self.spacing * context.scale;

		f32 mainAxisAllChildSize{ 0.0f };
		for (auto& child : self.children) {
			mainAxisAllChildSize += (self.bVertical ? manager.minsizes[child].y : manager.minsizes[child].x);
		}
		mainAxisAllChildSize *= context.scale;
		const f32 childCount = static_cast<f32>(self.children.size());
		const f32 allSpacing{ (childCount - 1) * scaledSpaceing };

		const f32 mainAxisLeftSpace = (self.bVertical ? scaledWholeSize.y : scaledWholeSize.x) - scaledSpaceing * (childCount - 1.0f) - mainAxisAllChildSize;
		const f32 perChildExtraSpace = mainAxisLeftSpace / childCount;

		DrawContext childContext = context;
		childContext.topleft = Vec2{ place.x - scaledHalfSize.x, place.y + scaledHalfSize.y };
		childContext.bottomright = Vec2{ place.x + scaledHalfSize.x, place.y - scaledHalfSize.y };
		childContext.xalign = self.xalign;
		childContext.yalign = self.yalign;
		fit(childContext, self.padding);

		f32 uniformSpacePerChild = ((self.bVertical ? childContext.size().y : childContext.size().x) - scaledSpaceing * (childCount - 1.0f)) / childCount;
		f32 offset{ 0.0f };	// the offset to the top or to the left of the context, that next child is getting its context shifted by
		if (self.packing == Packing::Tight) {
			const f32 mainAxisContextSize = (self.bVertical ? scaledWholeSize.y : scaledWholeSize.x);
			if (self.bVertical) {
				if (self.yalign == YAlign::Center)
					offset += (mainAxisContextSize - mainAxisAllChildSize - allSpacing) * 0.5f;
				if (self.yalign == YAlign::Bottom)
					offset += mainAxisContextSize - mainAxisAllChildSize - allSpacing;
			}
			else {
				if (self.xalign == XAlign::Center)
					offset += (mainAxisContextSize - mainAxisAllChildSize - allSpacing) * 0.5f;
				if (self.xalign == XAlign::Right)
					offset += mainAxisContextSize - mainAxisAllChildSize - allSpacing;
			}
		}

		for (auto& childid : self.children) {
			ElementVariant& childvar = manager.elements[childid];
			const auto scaledSize = manager.minsizes[childid] * context.scale;

			DrawContext myContext = childContext;
			if (self.bVertical) {
				if (self.packing == Packing::Tight) {
					myContext.cutTop(offset);
					myContext.cutBottom(childContext.size().y - scaledSize.y - offset);
					offset += scaledSize.y + scaledSpaceing;
				}
				else if (self.packing == Packing::Spread) {
					myContext.cutTop(offset);
					myContext.cutBottom(childContext.size().y - scaledSize.y - perChildExtraSpace - offset);
					offset += scaledSize.y + scaledSpaceing + perChildExtraSpace;
				}
				else /* self.packing == Packing::Uniform */ {
					myContext.cutTop(offset);
					myContext.cutBottom(childContext.size().y - uniformSpacePerChild - offset);
					offset += scaledSpaceing + uniformSpacePerChild;
				}
			}
			else {
				if (self.packing == Packing::Tight) {
					myContext.cutLeft(offset);
					myContext.cutRight(childContext.size().x - scaledSize.x - offset);
					offset += scaledSize.x + scaledSpaceing;
				}
				else if (self.packing == Packing::Spread) {
					myContext.cutLeft(offset);
					myContext.cutRight(childContext.size().x - scaledSize.x - perChildExtraSpace - offset);
					offset += scaledSize.x + scaledSpaceing + perChildExtraSpace;
				}
				else /* self.packing == Packing::Uniform */ {
					myContext.cutLeft(offset);
					myContext.cutRight(childContext.size().x - uniformSpacePerChild - offset);
					offset += scaledSpaceing + uniformSpacePerChild;
				}
			}
			draw(manager, childvar, childid, myContext, out);
		}
	}

	template<> inline Vec2 updateAndGetMinsize(Manager& manager, DragDroppable& self)
	{
		Vec2 minsize;
		if (self.child != INVALID_ELEMENT_ID) {
			minsize = updateAndGetMinsize(manager, self.child);
		}
		if (self.onUpdate) self.onUpdate(self);
		return minsize;
	}
	template<> inline void onDraw<DragDroppable>(Manager& manager, DragDroppable& self, u32 id, DrawContext const& context, std::vector<Sprite>& out)
	{
		const Vec2 scaledSize = manager.minsizes[id] * context.scale;
		Vec2 place = getPlace(scaledSize, context);

		const Vec2 cursor = manager.renderer->convertCoordSys(manager.window->getCursorPos(), RenderSpace::WindowSpace, RenderSpace::PixelSpace);
		const bool cursorOverElement = isCoordInBounds(scaledSize, place, cursor);

		if (cursorOverElement) {
			manager.requestMouseEvent(id, context.root, context.renderDepth);
		}

		const bool bDragged = manager.draggedElement.first == id;
		if (bDragged) {
			place = cursor;
		}

		const bool wasIDropped = manager.droppedElementId == id;
		/* when we are getting dropped, a potential catch could accur AFTER this draw, so the element will jump for one frame. To avoid that we do not draw when we are dropped */
		if (!wasIDropped) {	
			DrawContext childContext = context;
			if (bDragged) {
				fit(childContext, scaledSize, place);
				childContext.renderDepth -= 0.001f;
			}

			if (self.child != INVALID_ELEMENT_ID) {
				draw(manager, manager.elements[self.child], self.child, childContext, out);
			}
		}
	}

	template<> inline Vec2 updateAndGetMinsize(Manager& manager, DropBox& self)
	{
		Vec2 minsize;
		if (self.child != INVALID_ELEMENT_ID) {
			minsize = updateAndGetMinsize(manager, self.child);
		}
		if (self.onUpdate) self.onUpdate(self);
		minsize = max(minsize, self.minsize);
		return minsize;
	}
	template<> inline void onDraw<DropBox>(Manager& manager, DropBox& self, u32 id, DrawContext const& context, std::vector<Sprite>& out)
	{
		const Vec2 scaledSize = manager.minsizes[id] * context.scale;
		const Vec2 place = getPlace(scaledSize, context);

		const Vec2 cursor = manager.renderer->convertCoordSys(manager.window->getCursorPos(), RenderSpace::WindowSpace, RenderSpace::PixelSpace);
		const bool cursorOverElement = isCoordInBounds(scaledSize, place, cursor);

		if (cursorOverElement) {
			if (manager.window->buttonJustReleased(MouseButton::MB_LEFT) || self.bCatchMouseInput) {
				manager.requestMouseEvent(id, context.root, context.renderDepth);
			}
		}

		if (cast<Vec4&>(self.color).a != 0.0f) {
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

		if (self.child != INVALID_ELEMENT_ID) {
			auto childcontext = context;
			childcontext.xalign = XAlign::Center;
			childcontext.yalign = YAlign::Center;
			fit(childcontext, scaledSize, place);
			draw(manager, manager.elements[self.child], self.child, childcontext, out);
		}
	}

	template<> inline Vec2 updateAndGetMinsize(Manager& manager, Footer& self)
	{
		Vec2 minsize;
		for (u32 child : self.children) {
			auto childMinSize = updateAndGetMinsize(manager, child);
			minsize.x = std::max(minsize.x, childMinSize.x);
			minsize.y += childMinSize.y;
		}
		if (self.onUpdate) self.onUpdate(self);
		minsize.y += self.spacing;
		return minsize;
	}
	template<> inline void onDraw<Footer>(Manager& manager, Footer& self, u32 id, DrawContext const& context, std::vector<Sprite>& out)
	{
		const f32 scaledFooterSize = self.size * context.scale;
		const f32 scaledSpacing = self.spacing * context.scale;
		const f32 scaledHeadderSize = context.size().y - scaledFooterSize - scaledSpacing;

		if (self.children.size() >= 1) {
			const u32 headChild = self.children[0];
			auto headContext = context;
			headContext.cutBottom(scaledSpacing + scaledFooterSize);

			draw(manager, manager.elements[headChild], headChild, headContext, out);

			if (self.children.size() >= 2) {
				const u32 footerChild = self.children[1];
				auto footerContext = context;
				footerContext.cutTop(scaledHeadderSize + scaledSpacing);

				draw(manager, manager.elements[footerChild], footerChild, footerContext, out);
			}
		}
		if (self.children.size() != 2) {
			std::cerr << "WARNING: footer (id: "<<id<<") element does not have two children!\n";
		}
	}
}
