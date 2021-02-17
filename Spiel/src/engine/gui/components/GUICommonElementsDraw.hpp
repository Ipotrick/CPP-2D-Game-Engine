#pragma once

#include "GUIDraw.hpp"

namespace gui {

	template<> inline Vec2 updateAndGetMinsize<Box>(Manager& manager, u32 id, Box& self)
	{
		if (self.onUpdate) self.onUpdate(self, id);
		Vec2 minsize;
		if (self.child != INVALID_ELEMENT_ID) {
			minsize = updateAndGetMinsize(manager, self.child);
		}
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
					.cornerRounding = 3.0f * context.scale,
					.drawMode = RenderSpace::Pixel
				}
			);
		}

		if (self.bDragable) {
			const Vec2 cursor = manager.coordSys.convertCoordSys(manager.window->getCursorPos(), RenderSpace::Window, RenderSpace::Pixel);
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
	
	template<> inline Vec2 updateAndGetMinsize<_Button>(Manager& manager, u32 id, _Button& self)
	{
		if (self.onUpdate) self.onUpdate(self, id);
		if (self.child != INVALID_ELEMENT_ID) updateAndGetMinsize(manager, self.child);
		return self.size;
	}
	template<> inline void onDraw<_Button>(Manager& manager, _Button& self, u32 id, DrawContext const& context, std::vector<Sprite>& out)
	{
		auto scaledSize = self.size * context.scale;
		auto place = getPlace(scaledSize, context);

		Vec2 cursor = manager.coordSys.convertCoordSys(manager.window->getCursorPos(), RenderSpace::Window, RenderSpace::Pixel);
		const bool cursorOverElement = isCoordInBounds(scaledSize, place, cursor);

		if (cursorOverElement) {
			manager.requestMouseEvent(id, context.root, context.renderDepth);
		}
		else {
			self.bHold = false;
			self.bHover = false;
		}

		f32 extraSizeScale = (self.bHover ? 0.95 : 1.0f);

		out.push_back(
			Sprite{
				.color = self.bHold ? self.holdColor : self.color,
				.position = Vec3{place, context.renderDepth },
				.scale = scaledSize * extraSizeScale,
				.cornerRounding = 3.0f * context.scale,
				.drawMode = RenderSpace::Pixel
			}
		);


		if (self.child != INVALID_ELEMENT_ID) {
			const bool validChild = manager.isOneOf<Text, StaticText>(self.child);
			assert(validChild);
			auto childcontext = context;
			fit(childcontext, scaledSize, place);
			childcontext.xalign = XAlign::Center;
			childcontext.yalign = YAlign::Center;
			childcontext.scale *= extraSizeScale;
			draw(manager, manager.elements[self.child], self.child, childcontext, out);
		}
	}

	template<> inline Vec2 updateAndGetMinsize(Manager& manager, u32 id, _Checkbox& self)
	{
		if (self.onUpdate) self.onUpdate(self, id);
		return self.size;
	}
	template<> inline void onDraw<_Checkbox>(Manager& manager, _Checkbox& self, u32 id, DrawContext const& context, std::vector<Sprite>& out)
	{
		Vec2 scaledSize = self.size * context.scale;
		Vec2 place = getPlace(scaledSize, context);

		const Vec2 cursor = manager.coordSys.convertCoordSys(manager.window->getCursorPos(), RenderSpace::Window, RenderSpace::Pixel);
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
				.cornerRounding = 2.0f * context.scale,
				.drawMode = RenderSpace::Pixel
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
					.cornerRounding = 1.5f * context.scale,
					.drawMode = RenderSpace::Pixel
				}
			);
		}
	}

	template<> inline Vec2 updateAndGetMinsize<SliderF64>(Manager& manager, u32 id, SliderF64& self)
	{
		if (self.onUpdate) self.onUpdate(self, id);
		if (self.child != INVALID_ELEMENT_ID) updateAndGetMinsize(manager, self.child);
		return self.size;
	}
	template<> inline void onDraw<SliderF64>(Manager& manager, SliderF64& self, u32 id, DrawContext const& context, std::vector<Sprite>& out)
	{
		const Vec2 responsiveArea = self.size * context.scale;
		const Vec2 place = getPlace(responsiveArea, context);
		const Vec2 scaledBarSize = responsiveArea * (self.bThin ? (self.bVertical ? Vec2{ 0.225f, 1.0f } : Vec2{ 1.0f, 0.225f }) : Vec2{ 1,1 });
		const f32 barThiccness = std::min(scaledBarSize.x, scaledBarSize.y);

		const f32 biggerInidcatorSize = std::min(responsiveArea.x, responsiveArea.y);
		const f32 smallerInidcatorSize = biggerInidcatorSize * (self.bThin ? 1.0f : 0.2f);
		const Vec2 indicatorSize2 = (self.bVertical ? Vec2{ biggerInidcatorSize, smallerInidcatorSize } : Vec2{ smallerInidcatorSize, biggerInidcatorSize });

		const bool rangeOK{ self.max > self.min };
		// Draw bar:
		out.push_back(
			Sprite{
				.color = rangeOK ? self.colorBar : self.colorError,
				.position = Vec3{place, context.renderDepth},
				.scale = scaledBarSize,
				.cornerRounding = biggerInidcatorSize * 0.2f * 0.5f,
				.drawMode = RenderSpace::Pixel
			}
		);

		const Vec2 cursor = manager.coordSys.convertCoordSys(manager.window->getCursorPos(), RenderSpace::Window, RenderSpace::Pixel);
		if (isCoordInBounds(responsiveArea, place, cursor)) {
			manager.requestMouseEvent(id, context.root, context.renderDepth);
		}

		auto drawInvalidSlider = [&] () {  
			out.push_back(
				Sprite{
					.color = rangeOK ? self.colorBar : self.colorError,
					.position = Vec3{place, context.renderDepth},
					.scale = indicatorSize2,
					.cornerRounding = smallerInidcatorSize * 0.5f,
					.drawMode = RenderSpace::Pixel
				}
			);
		};

		if (self.value) {
			const f32 sliderMinPosMainAxis	= (self.bVertical ? place.y - scaledBarSize.y * 0.5f : place.x - scaledBarSize.x * 0.5f) + smallerInidcatorSize * 0.5f;
			const f32 sliderMaxPosMainAxis	= (self.bVertical ? place.y + scaledBarSize.y * 0.5f : place.x + scaledBarSize.x * 0.5f) - smallerInidcatorSize * 0.5f;
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
						.scale = indicatorSize2,
						.cornerRounding = smallerInidcatorSize * 0.5f,
						.drawMode = RenderSpace::Pixel
					}
				);

				if (self.child != INVALID_ELEMENT_ID) {
					const bool validChild = manager.isOneOf<Text, StaticText>(self.child);
					assert(validChild && !self.bVertical && !self.bThin);
					auto childcontext = context;
					fit(childcontext, scaledBarSize, place);
					childcontext.cutLeft(smallerInidcatorSize);
					childcontext.cutRight(smallerInidcatorSize);
					childcontext.xalign = XAlign::Center;
					childcontext.yalign = YAlign::Center;
					draw(manager, manager.elements[self.child], self.child, childcontext, out);
				}
			}
		}
		else {
			drawInvalidSlider();
		}
	}

	template<> inline Vec2 updateAndGetMinsize(Manager& manager, u32 id, Group& self)
	{
		if (self.onUpdate) { self.onUpdate(self, id); }
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

	template<> inline Vec2 updateAndGetMinsize(Manager& manager, u32 id, DragDroppable& self)
	{
		if (self.onUpdate) self.onUpdate(self, id);
		Vec2 minsize;
		if (self.child != INVALID_ELEMENT_ID) {
			minsize = updateAndGetMinsize(manager, self.child);
		}
		return minsize;
	}
	template<> inline void onDraw<DragDroppable>(Manager& manager, DragDroppable& self, u32 id, DrawContext const& context, std::vector<Sprite>& out)
	{
		const Vec2 scaledSize = manager.minsizes[id] * context.scale;
		Vec2 place = getPlace(scaledSize, context);

		const Vec2 cursor = manager.coordSys.convertCoordSys(manager.window->getCursorPos(), RenderSpace::Window, RenderSpace::Pixel);
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

	template<> inline Vec2 updateAndGetMinsize(Manager& manager, u32 id, DropBox& self)
	{
		if (self.onUpdate) self.onUpdate(self, id);
		Vec2 minsize;
		if (self.child != INVALID_ELEMENT_ID) {
			minsize = updateAndGetMinsize(manager, self.child);
		}
		minsize = max(minsize, self.minsize);
		return minsize;
	}
	template<> inline void onDraw<DropBox>(Manager& manager, DropBox& self, u32 id, DrawContext const& context, std::vector<Sprite>& out)
	{
		const Vec2 scaledSize = manager.minsizes[id] * context.scale;
		const Vec2 place = getPlace(scaledSize, context);

		const Vec2 cursor = manager.coordSys.convertCoordSys(manager.window->getCursorPos(), RenderSpace::Window, RenderSpace::Pixel);
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
					.cornerRounding = 3.0f * context.scale,
					.drawMode = RenderSpace::Pixel
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

	template<> inline Vec2 updateAndGetMinsize(Manager& manager, u32 id, Footer& self)
	{
		if (self.onUpdate) self.onUpdate(self, id);
		Vec2 minsize;
		for (u32 child : self.children) {
			auto childMinSize = updateAndGetMinsize(manager, child);
			minsize.x = std::max(minsize.x, childMinSize.x);
			minsize.y += childMinSize.y;
		}
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
