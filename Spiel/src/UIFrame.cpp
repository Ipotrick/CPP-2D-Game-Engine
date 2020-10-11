#include "UIFrame.hpp"

void UIFrame::draw(std::vector<Drawable>& buffer, UIContext context)
{
	context.drawMode =	this->drawMode;
	Vec2 size =			this->size * context.scale;
	Vec2 position =		this->anchor.getOffset(size, context);
	Vec2 borders =		this->borders * context.scale;
	drawCorners(	buffer, context, position, size, borders);
	drawBorders(	buffer, context, position, size, borders);
	drawFill(		buffer, context, position, size, borders);
	drawChildren(	buffer, context, position, size, borders);

	lastDrawArea = anchor.shrinkContextToMe(this->size, context);
}

void UIFrame::drawCorners(std::vector<Drawable>& buffer, UIContext context, const Vec2 position, const Vec2 size, const Vec2 borders)
{
	auto cornerDrawable = Drawable(0, Vec2{ 0, 0 }, context.drawingPrio, borders, Vec4{ 1,1,1,1 }, Form::Rectangle, RotaVec2(0), context.drawMode, texAtlas);
	// upper left corner:
	cornerDrawable.position = { position + Vec2(-size.x, size.y) * 0.5f + Vec2(borders.x, -borders.y) * 0.5f };
	cornerDrawable.texRef.value().minPos = { 0.0f, 0.5f };
	cornerDrawable.texRef.value().maxPos = { 0.5f, 1.0f };
	buffer.push_back(cornerDrawable);
	// lower left corner:
	cornerDrawable.position = { position + Vec2(-size.x, -size.y) * 0.5f + Vec2(borders.x, borders.y) * 0.5f };
	cornerDrawable.texRef.value().minPos = { 0.0f, 1.0f };
	cornerDrawable.texRef.value().maxPos = { 0.5f, 0.5f };
	buffer.push_back(cornerDrawable);
	// lower right corner:
	cornerDrawable.position = { position + Vec2(size.x, -size.y) * 0.5f + Vec2(-borders.x, borders.y) * 0.5f };
	cornerDrawable.texRef.value().minPos = { 0.5f, 1.0f };
	cornerDrawable.texRef.value().maxPos = { 0.0f, 0.5f };
	buffer.push_back(cornerDrawable);
	// upper right corner:
	cornerDrawable.position = { position + Vec2(size.x, size.y) * 0.5f + Vec2(-borders.x, -borders.y) * 0.5f };
	cornerDrawable.texRef.value().minPos = { 0.5f, 0.5f };
	cornerDrawable.texRef.value().maxPos = { 0.0f, 1.0f };
	buffer.push_back(cornerDrawable);
}

void UIFrame::drawBorders(std::vector<Drawable>& buffer, UIContext context, const Vec2 position, const Vec2 size, const Vec2 borders)
{
	// draw vertical borders:
	float borderLen = size.y - borders.y - borders.y;
	auto borderDrawable = Drawable(0, { 0,0 }, context.drawingPrio, Vec2{ borders.x, borderLen }, Vec4{ 1,1,1,1 }, Form::Rectangle, RotaVec2(0), context.drawMode, texAtlas);
	// left border:
	borderDrawable.position = position + Vec2{ -size.x + borders.x, 0.0f } *0.5f;
	borderDrawable.texRef.value().maxPos = { 0.5f, 0.5f };
	borderDrawable.texRef.value().minPos = { 0.0f, 0.0f };
	buffer.push_back(borderDrawable);
	// right border:
	borderDrawable.position = position + Vec2(size.x - borders.x, 0) * 0.5f;
	borderDrawable.texRef.value().maxPos = { 0.0f, 0.5f };
	borderDrawable.texRef.value().minPos = { 0.5f, 0.0f };
	buffer.push_back(borderDrawable);

	// draw vertical borders:
	borderLen = size.x - borders.x - borders.x;
	borderDrawable = Drawable(0, Vec2{ 0,0 }, context.drawingPrio, Vec2{ borderLen, borders.y }, Vec4{ 1,1,1,1 }, Form::Rectangle, RotaVec2(0), context.drawMode, texAtlas);
	// upper border:
	borderDrawable.position = position + Vec2(0, size.y - borders.y) * 0.5f;
	borderDrawable.texRef.value().maxPos = Vec2(1.0f, 1.0f);
	borderDrawable.texRef.value().minPos = Vec2(0.5f, 0.5f);
	buffer.push_back(borderDrawable);
	// lower border:
	borderDrawable.position = position + Vec2(0, -size.y + borders.y) * 0.5f;
	borderDrawable.texRef.value().maxPos = Vec2(1.0f, 0.5f);
	borderDrawable.texRef.value().minPos = Vec2(0.5f, 1.0f);
	buffer.push_back(borderDrawable);
}

void UIFrame::drawFill(std::vector<Drawable>& buffer, UIContext context, const Vec2 position, const Vec2 size, const Vec2 borders)
{
	SmallTextureRef fillTex = texAtlas;
	fillTex.maxPos = Vec2(1.0f, 0.5f);
	fillTex.minPos = Vec2(0.5f, 0.0f);
	buffer.push_back(
		Drawable(0, position, context.drawingPrio, Vec2{ size.x - borders.x - borders.x, size.y - borders.y - borders.y }, Vec4{ 1,1,1,1 }, Form::Rectangle, RotaVec2(0), context.drawMode, fillTex));
}

void UIFrame::drawChildren(std::vector<Drawable>& buffer, UIContext context, const Vec2 position, const Vec2 size, const Vec2 borders)
{
	if (hasChild()) {
		context.ulCorner = position + Vec2(1, -1) * (0.5f * -size + borders);
		context.drCorner = position - Vec2(1, -1) * (0.5f * -size + borders);
		context.increaseDrawPrio();
		getChild()->draw(buffer, context);
	}
}
