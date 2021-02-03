#pragma once

#include "GUIDrawContext.hpp"
#include "../../rendering/RenderTypes.hpp"

namespace gui {

	struct TextContext : public DrawContext {
		TextContext(DrawContext const& super) : DrawContext{ super } {}
		Vec2 fontSize;
		Vec4 color{ 0,0,0,1 };
		TextureRef texref;
	};

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
		const Vec2 SCALED_FONT_SIZE = context.fontSize * context.scale;
		const float MAX_COLS = std::floorf(context.size().x / SCALED_FONT_SIZE.x + 0.0001f);
		const float MAX_ROWS = std::floorf(context.size().y / SCALED_FONT_SIZE.y + 0.0001f);
		Vec2 START_PLACE = context.centerpos() + Vec2{ -(MAX_COLS - 1.0f) * 0.5f * SCALED_FONT_SIZE.x, (MAX_ROWS - 1.0f) * 0.5f * SCALED_FONT_SIZE.y };

		TextureRef tex = context.texref;

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

				tex.minPos = atlasCoord / ATLAS_GRID_SIZE;
				tex.maxPos = atlasCoord / ATLAS_GRID_SIZE + ATLAS_CELL_SIZE;

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
						.scale = SCALED_FONT_SIZE,
						.textureId = tex.id,
						.minTex = tex.minPos,
						.maxTex = tex.maxPos,
						.form = Form::Rectangle,
						.drawMode = RenderSpace::PixelSpace,
					}
				);
				colIndex += 1;
				charsProcessed += 1;
			}
		}
		return charsProcessed;
	}

	inline Vec2 getMinSizeText(const char* str, Vec2 fontSize)
	{
		const char* iter = str;
		if (iter == nullptr || iter[0] == 0x00) return Vec2{ 0,0 };
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
		return Vec2{ maxcols * fontSize.x, rows * fontSize.y };
	}
}
