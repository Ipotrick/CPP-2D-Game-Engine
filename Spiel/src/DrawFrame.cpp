#include "DrawFrame.hpp"

void drawFrame(std::vector<Drawable>& buffer, UIContext context)
{

}

void drawFrame(std::vector<Drawable>& buffer, UIContext context, Vec2 pos, Vec2 size, Vec2 border, Vec4 borderColor, Vec4 fillColor)
{
	// fill:
	buffer.push_back(
		Drawable(0, pos, context.drawingPrio, size - border * 2.0f, fillColor, Form::Rectangle, RotaVec2(0), context.drawMode)
	);
	
	// borders:
	Vec2 bPos{ 0.0f, 0.0f };
	Vec2 bSize{ 0.0f, 0.0f };
	auto drawBorder = [&]() {
		buffer.push_back(Drawable(0, bPos, context.drawingPrio, bSize, borderColor, Form::Rectangle, RotaVec2(0), context.drawMode));
	};
	// left:
	bSize = Vec2{ border.x, size.y };
	bPos = pos - Vec2{ (size.x - border.x) * 0.5f, 0.0f };
	drawBorder();
	// right:
	bPos = pos + Vec2{ (size.x - border.x) * 0.5f, 0.0f };
	drawBorder();
	// top:
	bSize = Vec2{ size.x - border.x * 2.0f, border.y };
	bPos = pos + Vec2{ 0.0f, (size.y - border.y) * 0.5f };
	drawBorder();
	// bottom:
	bPos = pos - Vec2{ 0.0f, (size.y - border.y) * 0.5f };
	drawBorder();
}
