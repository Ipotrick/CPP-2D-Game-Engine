#include "UIFrame.hpp"

void UIFrame::draw(std::vector<Drawable>& buffer, UIContext context)
{
	context.drawMode = this->drawMode;
	drawCorners(buffer, context);
	drawBorders(buffer, context);
	drawFill(buffer, context);
	drawChildren(buffer, context);
}

void UIFrame::drawCorners(std::vector<Drawable>& buffer, UIContext context)
{
	auto cornerDrawable = Drawable(0, Vec2{ 0, 0 }, 1.0f, borders, Vec4{ 1,1,1,1 }, Form::Rectangle, RotaVec2(0), context.drawMode, texAtlas);
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

void UIFrame::drawBorders(std::vector<Drawable>& buffer, UIContext context)
{
	{
		// draw vertical borders:
		float borderLen = size.y - borders.y - borders.y;
		auto borderDrawable = Drawable(0, { 0,0 }, 1.0f, Vec2{ borders.x, borderLen }, Vec4{ 1,1,1,1 }, Form::Rectangle, RotaVec2(0), context.drawMode, texAtlas);
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
	}
	{
		// draw vertical borders:
		float borderLen = size.x - borders.x - borders.x;
		auto borderDrawable = Drawable(0, Vec2{ 0,0 }, 1.0f, Vec2{ borderLen, borders.y }, Vec4{ 1,1,1,1 }, Form::Rectangle, RotaVec2(0), context.drawMode, texAtlas);
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
}

void UIFrame::drawFill(std::vector<Drawable>& buffer, UIContext context)
{
	TextureRef fillTex = texAtlas;
	fillTex.maxPos = Vec2(1.0f, 0.5f);
	fillTex.minPos = Vec2(0.5f, 0.0f);
	buffer.push_back(
		Drawable(0, position, 1.0f, Vec2{ size.x - borders.x - borders.x, size.y - borders.y - borders.y }, Vec4{ 1,1,1,1 }, Form::Rectangle, RotaVec2(0), context.drawMode, fillTex));
}

void UIFrame::drawChildren(std::vector<Drawable>& buffer, UIContext context)
{
	if (hasChild()) {
		context.ulCorner = position + Vec2(1, -1) * (0.5f * -size + borders);
		context.drCorner = position - Vec2(1, -1) * (0.5f * -size + borders);
		context.scale = scaling;
		context.depth += 1;
		getChild()->draw(buffer, context);
	}
}
