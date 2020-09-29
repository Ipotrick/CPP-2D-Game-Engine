#pragma once

#include "UIElement.hpp"

class UIText : public UIElement {
public:

	UIText(std::string_view str, int fontAtlasId)
		:text{ str }, fontTextureId{ fontAtlasId }
	{

	}

	UIText() {}

	virtual void draw(std::vector<Drawable>& buffer, UIContext context) override
	{
		buffer.push_back(Drawable(0, context.ulCorner, 2.0f, Vec2(2, 2), Vec4(1, 0, 0, 1), Form::Rectangle, RotaVec2(0), context.drawMode));
		int column = 0;
		int row = 0;
		Vec2 size = context.scale * (Vec2(1, 1) * fontSize);
		auto letter = Drawable(0, Vec2(), 1.0f + 0.01f * context.depth, size, color, Form::Rectangle, RotaVec2(0), context.drawMode);
		for (auto c : text) {
			if (c == '\n') {
				row += 1;
				column = 0;
			}
			else {
				letter.position = context.ulCorner + context.scale * (Vec2(0.5f + column, -0.5f - row) * fontSize);
				if ((letter.position + Vec2(fontSize * 0.499f, 0)).x > context.drCorner.x) {	// overrun right border
					row += 1;
					column = 0;
					continue;
				}
				else {
					if ((letter.position - Vec2(0, fontSize * 0.499f)).y < context.drCorner.y) {	// overrun lower border
						break;
					}
				}

				letter.texRef = makeAsciiRef(fontTextureId, c);

				column += 1;
				buffer.push_back(letter);
			}
		}
	}

	std::string text{ "" };
	int fontTextureId{ -1 };
	Vec2 borderDist{ 0.0f, 0.0f };
	float fontSize{ 1.0f };
	Vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
private:
};