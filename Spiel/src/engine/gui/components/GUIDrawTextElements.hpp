#pragma once

#include "GUIDrawElements.hpp"

namespace gui {
	template<> inline Vec2 minsizeOf(Manager& manager, StaticText& self)
	{
		char const* iter = self.str;
		int rows{ 1 };
		int cols{ 0 };
		int maxcols{ 0 };
		while (*iter != 0x00) {
			if (*iter == '\n') {
				rows += 1;
				cols = 0;
			}
			else {
				cols += 1;
				maxcols = std::max(maxcols, cols);
			}
			++iter;
		}
		return Vec2{ maxcols * self.fontSize.x, rows * self.fontSize.y };
	}
	template<> inline void onDraw(Manager& manager, StaticText& self, u32 id, DrawContext const& context, std::vector<Sprite>& out)
	{
		if (!self.fonttexture.valid()) {
			manager.renderer->validateTextureRef(self.fonttexture);
		}

		auto scalesCompleteSize = manager.minsizes[id].value() * context.scale;
		Vec2 centerplace = getPlace(scalesCompleteSize, context);

		const float cols = manager.minsizes[id].value().x / self.fontSize.x;
		const float rows = manager.minsizes[id].value().y / self.fontSize.y;
		const Vec2 scaledFontSize = self.fontSize * context.scale;
		const Vec2 startplace = centerplace + Vec2{ -(cols - 1.0f) * 0.5f * scaledFontSize.x, (rows - 1.0f) * 0.5f * scaledFontSize.y };

		float colIndex = 0;
		float rowIndex = 0;
		for (const char* iter = self.str; *iter != 0x00; ++iter) {
			if (*iter == '\n') {
				rowIndex += 1;
				colIndex = 0;
			}
			else {
				const Vec2 place = startplace + Vec2{ colIndex, -rowIndex } * scaledFontSize;

				char c = *iter - 0x20;
				Vec2 atlasCoord{ float(c % 8), float(15 - c / 8) };
				constexpr Vec2 ATLAS_GRID_SIZE{ 8,16 };
				constexpr Vec2 ATLAS_CELL_SIZE{ 1.0f / 8.0f, 1.0f / 16.0f };

				self.fonttexture.minPos = atlasCoord / ATLAS_GRID_SIZE;
				self.fonttexture.maxPos = atlasCoord / ATLAS_GRID_SIZE + ATLAS_CELL_SIZE;

				out.push_back(
					Sprite{
						.color = Vec4{1,1,1,1},
						.position = Vec3{place, context.renderDepth},
						.scale = scaledFontSize,
						.texRef = self.fonttexture.makeSmall(),
						.form = Form::Rectangle,
						.drawMode = RenderSpace::PixelSpace,
					}
				);

				colIndex += 1;
			}
		}
	}

	template<> inline Vec2 minsizeOf(Manager& manager, _TextInput& self)
	{
		return self.size;
	}
	template<> inline void onDraw(Manager& manager, _TextInput& self, u32 id, DrawContext const& context, std::vector<Sprite>& out)
	{
		if (!self.fonttexture.valid()) {
			manager.renderer->validateTextureRef(self.fonttexture);
		}

		auto scalesCompleteSize = manager.minsizes[id].value() * context.scale;
		Vec2 centerplace = getPlace(scalesCompleteSize, context);

		bool cursorOverElement{ false };
		Vec2 cursor = manager.renderer->convertCoordSys(manager.window->getCursorPos(), RenderSpace::WindowSpace, RenderSpace::PixelSpace);
		Vec2 relCursor = cursor - centerplace;
		cursorOverElement =
			relCursor.x < scalesCompleteSize.x * 0.5f &&
			relCursor.y < scalesCompleteSize.y * 0.5f &&
			relCursor.x > -scalesCompleteSize.x * 0.5f &&
			relCursor.y > -scalesCompleteSize.y * 0.5f;

		if (cursorOverElement)
			manager.requestMouseEvent(id, context.root, context.renderDepth);

		Vec4 color{ 0.8,0.8,0.8,1 };
		const bool focused = manager.focusedTextInput.first == id;
		if (focused) {
			// we are the focused text input
			color = { 1,1,1,1 };
			if (manager.window->keyJustPressed(Key::ESCAPE)) {
				manager.focusedTextInput = { INVALID_ELEMENT_ID, {} };
			}

			auto keyEvents = manager.window->getKeyEventsInOrder();

			bool shift = manager.window->keyPressed(Key::LEFT_SHIFT);

			for (auto e : keyEvents) {
				if (e.type == KeyEvent::Type::JustPressed) {
					if (e.key == Key::BACKSPACE) {
						if (self.str.size() > 0) self.str.pop_back();
					}
					else if (e.key <= Key::Z && e.key >= Key::A) {
						self.str += cast<char>(e.key) + (shift ? 0x00 : 0x20);
						manager.window->consumeKeyEvent(e.key);
					}
					else if (e.key == Key::SPACE) {
						self.str += cast<char>(e.key);
						manager.window->consumeKeyEvent(e.key);
					}
					else if (e.key == Key::ENTER) {
						if (shift) {
							self.str += '\n';
							manager.window->consumeKeyEvent(e.key);
						}
						else {
							manager.focusedTextInput = { INVALID_ELEMENT_ID, {} };
							self.onStore(self.str);
							if (self.bClearOnEnter) self.str.clear();
						}
					}
					else if (e.key == Key::DELETE) {
						self.str.clear();
					}
				}
			}
		}
		out.push_back(Sprite{
			.color = color,
			.position = Vec3{centerplace, context.renderDepth},
			.scale = scalesCompleteSize,
			.form = Form::Rectangle,
			.cornerRounding = 3.0f * context.scale,
			.drawMode = RenderSpace::PixelSpace 
			}
		);

		const float MAX_COLS = std::floorf(manager.minsizes[id].value().x / self.fontSize.x);
		const float MAX_ROWS = std::floorf(manager.minsizes[id].value().y / self.fontSize.y);
		const Vec2 SCALED_FONT_SIZE = self.fontSize * context.scale;
		const Vec2 START_PLACE = centerplace + Vec2{ -(MAX_COLS - 1.0f) * 0.5f * SCALED_FONT_SIZE.x, (MAX_ROWS - 1.0f) * 0.5f * SCALED_FONT_SIZE.y };

		float colIndex = 0;
		float rowIndex = 0;
		size_t charsProcessed = 0;
		for (char c : self.str) {
			if (rowIndex >= MAX_ROWS) /* out of rows */ {
				break;
			}
			if (c == '\n') /* line break */ {
				rowIndex += 1;
				colIndex = 0;
				charsProcessed += 1;
			}
			else /* draw character */{
				if (colIndex >= MAX_COLS) {
					rowIndex += 1;
					colIndex = 0;
					if (rowIndex >= MAX_ROWS) break;
				}
				c -= 0x20;
				Vec2 atlasCoord{ float(c % 8), float(15 - c / 8) };
				constexpr Vec2 ATLAS_GRID_SIZE{ 8,16 };
				constexpr Vec2 ATLAS_CELL_SIZE{ 1.0f / 8.0f, 1.0f / 16.0f };

				self.fonttexture.minPos = atlasCoord / ATLAS_GRID_SIZE;
				self.fonttexture.maxPos = atlasCoord / ATLAS_GRID_SIZE + ATLAS_CELL_SIZE;

				const Vec2 place = START_PLACE + Vec2{ colIndex, -rowIndex } *SCALED_FONT_SIZE;
				out.push_back(
					Sprite{
						.color = Vec4{0,0,0,1},
						.position = Vec3{ place, context.renderDepth },
						.scale = SCALED_FONT_SIZE,
						.texRef = self.fonttexture.makeSmall(),
						.form = Form::Rectangle,
						.drawMode = RenderSpace::PixelSpace,
					}
				);
				colIndex += 1;
				charsProcessed += 1;
			}
		}

		if (self.str.size() > charsProcessed) /* we stopped char processing cause the string is too long, we need to remove the last few chars */{
			self.str = self.str.substr(0, charsProcessed);
		}
	}
}
