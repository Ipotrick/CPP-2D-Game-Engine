#pragma once

#include "GUIDrawContext.hpp"
#include "../../rendering/Sprite.hpp"
#include "../../rendering/OpenGLAbstraction/OpenGLTexture.hpp"
#include "../../rendering/Font.hpp"

namespace gui {

	struct TextContext : public DrawContext {
		TextContext(DrawContext const& super) : DrawContext{ super } {}
		f32 fontSize;
		Vec4 color{ 0,0,0,1 };
		TextureHandle tex;
		Font const* font{nullptr};
		bool bPixelPerfectAlignment{ true };
	};

	inline u32 drawFontText(const char* str, TextContext const& context, std::vector<Sprite>& out)
	{
		if (str == nullptr || str[0] == 0x00) return 0;

		const Font& FONT = *context.font;

		f32 SCALED_FONT_SIZE =  context.fontSize * context.scale;
		if (context.bPixelPerfectAlignment) {
			SCALED_FONT_SIZE = std::round(SCALED_FONT_SIZE);
		}

		const f32 MAX_X = context.bottomright.x + 0.1f;
		const f32 MIN_Y = context.bottomright.y - 0.1f;

		const Glyph DUMMY;

		const f32 TEXT_UPLIFT = context.bPixelPerfectAlignment ? std::ceil(SCALED_FONT_SIZE / 4.0f) : SCALED_FONT_SIZE / 4.0f;

		std::vector<f32> rowWidths{0};
		rowWidths.reserve(10);
		for (const char* iter = str; *iter != 0x00; ++iter) {
			if (*iter == '\n') {
				rowWidths.push_back(0);
			}
			else {
				const Glyph scaledGlyph = scaleGlyph(FONT.codepointToGlyph.get(*iter), 4096, 4096);
				if (rowWidths.back() + scaledGlyph.advance * SCALED_FONT_SIZE < MAX_X) {
					rowWidths.back() += scaledGlyph.advance * SCALED_FONT_SIZE;
				}
			}
		}
		const f32 SCALED_COMPLETE_HEIGHT = rowWidths.size() * SCALED_FONT_SIZE;

		const f32 REMAINING_Y_SIZE = std::max(context.size().y - SCALED_COMPLETE_HEIGHT, 0.0f);
		f32 CURSOR_STARTING_Y = context.topleft.y - SCALED_FONT_SIZE; 
		if (context.yalign != YAlign::Top) {
			CURSOR_STARTING_Y -= REMAINING_Y_SIZE * (context.yalign == YAlign::Center ? 0.5f : 1.0f);
		}

		auto getStartingCursor = [&](u32 rowIndex) {
			const f32 REMAINING_X_SIZE_IN_ROW = std::max(context.size().x - rowWidths[rowIndex],0.0f);
			f32 CURSOR_STARTING_X = context.topleft.x;
			if (context.xalign != XAlign::Left) {
				CURSOR_STARTING_X += REMAINING_X_SIZE_IN_ROW * (context.xalign == XAlign::Center ? 0.5f : 1.0f);
			}

			return Vec2{ CURSOR_STARTING_X, CURSOR_STARTING_Y - rowIndex * SCALED_FONT_SIZE };
		};

		Vec2 cursor = getStartingCursor(0);
		u32 rowIndex{ 0 };
		u32 charsProcessed{ 0 };
		auto attemptLinebreak = [&]() -> bool {
			if (cursor.y - SCALED_FONT_SIZE > MIN_Y) {
				rowIndex += 1;
				if (rowIndex >= rowWidths.size()) return false;
				cursor = getStartingCursor(rowIndex);
				return true;
			}
			return false;
		};
		for (const char* iter = str; *iter != 0x00; ++iter) {
			if (context.bPixelPerfectAlignment) {
				cursor = ceil(cursor - Vec2{ 0.1f,0.1f });
			}
			if (*iter == '\n') {
				if (!attemptLinebreak()) break;
			} 
			else {
				const Glyph scaledGlyph = scaleGlyph(FONT.codepointToGlyph.get(*iter), 4096, 4096);

				if (scaledGlyph.advance * SCALED_FONT_SIZE + cursor.x < MAX_X) {

					Vec2 size = SCALED_FONT_SIZE * 
						Vec2{
							scaledGlyph.planeBounds.right-scaledGlyph.planeBounds.left,
							scaledGlyph.planeBounds.top-scaledGlyph.planeBounds.bottom
						};
					Vec2 place = cursor + Vec2{0, TEXT_UPLIFT }/* move text up by 25% */ +
						Vec2{
							SCALED_FONT_SIZE * scaledGlyph.planeBounds.left + size.x * 0.5f,
							SCALED_FONT_SIZE * scaledGlyph.planeBounds.bottom + size.y * 0.5f
						};
					out.push_back(
						Sprite{
							.color = context.color,
							.position = Vec3{ place, context.renderDepth },
							.scale = size,
							.texHandle = context.tex,
							.texMin = Vec2{scaledGlyph.atlasBounds.left, scaledGlyph.atlasBounds.bottom},
							.texMax = Vec2{scaledGlyph.atlasBounds.right, scaledGlyph.atlasBounds.top},
							.isMSDF = true,
							.drawMode = RenderSpace::Pixel,
						}
					);

					cursor.x += SCALED_FONT_SIZE * scaledGlyph.advance;
				}
				else {
					if (!attemptLinebreak()) break;
				}
			}
			charsProcessed += 1;
		}
		return charsProcessed;
	}

	/**
	 * \param out vector containing the resulting sprites.
	 * \param str text in ascii that should be drawn.
	 * \param context text information.
	 * \return amount of caracters actually drawn.
	 */
	inline u32 drawText(const char* str, TextContext const& context, std::vector<Sprite>& out)
	{
		if (str == nullptr || str[0] == 0x00) return 0;
		// TODO Left-, Center-, Right-alignment
		const f32 SCALED_FONT_SIZE = context.fontSize * context.scale;
		const float MAX_COLS = std::floorf(context.size().x / SCALED_FONT_SIZE + 0.0001f);
		const float MAX_ROWS = std::floorf(context.size().y / SCALED_FONT_SIZE + 0.0001f);
		Vec2 START_PLACE = context.centerpos() + Vec2{ -(MAX_COLS - 1.0f) * 0.5f * SCALED_FONT_SIZE, (MAX_ROWS - 1.0f) * 0.5f * SCALED_FONT_SIZE };

		TextureHandle tex = context.tex;

		std::vector<u32> colsPerRow{ 0 };
		for (const char* iter = str; *iter != 0x00; ++iter) {
			if (*iter == '\n' || colsPerRow.back() >= MAX_COLS /* wrapping */) {
				colsPerRow.emplace_back(0);
			}
			else {
				colsPerRow.back()++;
			}
		}

		float colIndex = 0;
		float rowIndex = 0;
		u32 charsProcessed = 0;
		for (const char* iter = str; *iter != 0x00; ++iter) {
			char c = *iter;
			if (rowIndex >= MAX_ROWS) /* out of rows */ {
				break;
			}
			if (c == '\n') /* line break */ {
				rowIndex += 1;
				colIndex = 0;
				charsProcessed += 1;
			}
			else /* draw character */ {
				if (colIndex >= MAX_COLS) {
					rowIndex += 1;
					colIndex = 0;
					if (rowIndex >= MAX_ROWS) break;
				}
				c -= 0x20;
				Vec2 atlasCoord{ float(c % 8), float(15 - c / 8) };
				constexpr Vec2 ATLAS_GRID_SIZE{ 8,16 };
				constexpr Vec2 ATLAS_CELL_SIZE{ 1.0f / 8.0f, 1.0f / 16.0f };

				auto texminPos = atlasCoord / ATLAS_GRID_SIZE;
				auto texmaxPos = atlasCoord / ATLAS_GRID_SIZE + ATLAS_CELL_SIZE;

				float rowStart = 0.0f;
				switch (context.xalign) {
				case XAlign::Left: rowStart = 0.0f; break;
				case XAlign::Center: rowStart = (MAX_COLS - colsPerRow[rowIndex]) * 0.5f; break;
				case XAlign::Right: rowStart = MAX_COLS - colsPerRow[rowIndex]; break;
				}
				const Vec2 place = 
					START_PLACE + 
					Vec2{ colIndex + rowStart, -rowIndex } *
					SCALED_FONT_SIZE;
				out.push_back(
					Sprite{
						.color = context.color,
						.position = Vec3{ place, context.renderDepth },
						.scale = Vec2{SCALED_FONT_SIZE, SCALED_FONT_SIZE},
						.texHandle = tex,
						.texMin = texminPos,
						.texMax = texmaxPos,
						.drawMode = RenderSpace::Pixel,
					}
				);
				colIndex += 1;
				charsProcessed += 1;
			}
		}
		return charsProcessed;
	}

	inline Vec2 getMinSizeText(const char* str, f32 fontSize, const Font& font)
	{
		const char* iter = str;
		if (iter == nullptr || iter[0] == 0x00) return Vec2{ 0,0 };
		int rows{ 1 };
		f32 maxwidth{ 0 };
		f32 currWidth{ 0 };
		while (*iter != 0x00) {
			if (*iter == '\n') {
				rows += 1;
				currWidth = 0.0f;
			}
			else {
				currWidth += font.codepointToGlyph.get(*iter).advance;
				maxwidth = std::max(maxwidth, currWidth);
			}
			++iter;
		}
		return Vec2{ maxwidth, f32(rows) } * fontSize;
	}
}
