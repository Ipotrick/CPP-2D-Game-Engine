#pragma once

#include "../../rendering/RenderTypes.hpp"

#include "GUIDrawContext.hpp"

namespace gui {

	/**
	 * \param sizeing defines the size paramters relative to the given context parameter.
	 * \param context is the relative frame ans scaling used to determine the size.
	 * \return size SCALED by the context.scale.
	 */
	static constexpr Vec2 getSize(const Sizeing& sizeing, const DrawContext& context)
	{
		return Vec2{
			sizeing.xmode == Sizeing::Mode::Absolute ? sizeing.x * context.scale : sizeing.x * context.size().x,
			sizeing.ymode == Sizeing::Mode::Absolute ? sizeing.y * context.scale : sizeing.y * context.size().y,
		};
	}

	/**
	 * \param size must be allready scaled by the given context.
	 * \return place generated by given parameters, scaled by the context.scale.
	 */
	static constexpr Vec2 getPlace(const Vec2 scaledSize, const Placeing& placeing, const DrawContext& context)
	{
		Vec2 place{ 0,0 };
		switch (placeing.xmode) {
		case Placeing::XMode::AbsoluteLeft:
			place.x = context.left() + scaledSize.x * 0.5f + placeing.x * context.scale; break;
		case Placeing::XMode::AbsoluteRight:
			place.x = context.right() - scaledSize.x * 0.5f - placeing.x * context.scale; break;
		case Placeing::XMode::RelativeLeft:
			place.x = context.left() + scaledSize.x * 0.5f + placeing.x * (context.size().x - scaledSize.x); break;
		case Placeing::XMode::RelativeRight:
			place.x = context.right() - scaledSize.x * 0.5f - placeing.x * (context.size().y - scaledSize.x); break;
		default: break;
		};
		switch (placeing.ymode) {
		case Placeing::YMode::AbsoluteTop:
			place.y = context.top() - scaledSize.y * 0.5f - placeing.y * context.scale; break;
		case Placeing::YMode::AbsoluteBottom:
			place.y = context.bottom() + scaledSize.y * 0.5f + placeing.y * context.scale; break;
		case Placeing::YMode::RelativeTop:
			place.y = context.top() - scaledSize.y * 0.5f - placeing.y * (context.size().y - scaledSize.y); break;
		case Placeing::YMode::RelativeBottom:
			place.y = context.bottom() + scaledSize.y * 0.5f + placeing.y * (context.size().y - scaledSize.y); break;
		default: break;
		};
		return place;
	}

	/**
	 * \param size must be allready scaled by the given context.
	 * \param context contains the alignment and the bounds, wich are used to determine the placeing.
	 * \return place generated by given parameters, scaled by the context.scale.
	 */
	static constexpr float getXPlace(const Vec2 scaledSize, const DrawContext& context)
	{
		float place{ 0.0f };
		switch(context.xalign) {
		case XAlign::Left:
			place = context.left() + scaledSize.x * 0.5f; break;
		case XAlign::Right:
			place = context.right() - scaledSize.x * 0.5f; break;
		case XAlign::Center:
			place = context.centerpos().x; break;
		default:
			assert(false);	// all other alignments are "faked" and must be replaced by the alignments above
		};
		return place;
	}

	/**
	 * \param size must be allready scaled by the given context.
	 * \param context contains the alignment and the bounds, wich are used to determine the placeing.
	 * \return place generated by given parameters, scaled by the context.scale.
	 */
	static constexpr float getYPlace(const Vec2 scaledSize, const DrawContext& context)
	{
		float place{ 0.0f };
		switch (context.yalign) {
		case YAlign::Top:
			place = context.top() - scaledSize.y * 0.5f; break;
		case YAlign::Bottom:
			place = context.bottom() + scaledSize.y * 0.5f; break;
		case YAlign::Center:
			place = context.centerpos().y; break;
		default:
			assert(false);	// all other alignments are "faked" and must be replaced by the alignments above
		};
		return place;
	}
	/**
	 * \param context contains the alignmentand the bounds, wich are used to determine the placeing.
	 * \return place generated by given parameters, scaled by the context.scale.
	 */
	static constexpr Vec2 getPlace(const Vec2 scaledSize, const DrawContext& context)
	{
		return Vec2{ getXPlace(scaledSize, context), getYPlace(scaledSize, context) };
	}

	/**
	 * \return scaled place relative to the context and sizeing, scaled by context.scale.
	 */
	static constexpr Vec2 getPlace(const Sizeing& sizeing, const Placeing& placeing, const DrawContext& context)
	{
		return getPlace(getSize(sizeing, context), placeing, context);
	}

	/**
	 * \return the position of the top left corner of the area generated by and object with the given placeing and sizeing in the given context.
	 */
	static constexpr Vec2 getTLCorner(const Sizeing& sizeing, const Placeing& placeing, const DrawContext& context)
	{
		const Vec2 size = getSize(sizeing, context);
		const Vec2 place = getPlace(size, placeing, context);
		return Vec2{ place.x - size.x * 0.5f, place.y + size.y * 0.5f };
	}

	/**
	 * \return the position of the bottom right corner of the area generated by and object with the given placeing and sizeing in the given context.
	 */
	static constexpr Vec2 getBRCorner(const Sizeing& sizeing, const Placeing& placeing, const DrawContext& context)
	{
		const Vec2 size = getSize(sizeing, context);
		const Vec2 place = getPlace(size, placeing, context);
		return Vec2{ place.x + size.x * 0.5f, place.y - size.y * 0.5f };
	}

	/**
	 * \return 4 rectangles representing the corners of the bounds of the given context.
	 */
	inline static std::array<Sprite, 4> debugContextCorners(DrawContext const& context)
	{
		return {
			Sprite{
				.color = Vec4{1,0.5,1,1},
				.position = {context.topleft, context.renderDepth},
				.scale = {5,5},
				.drawMode = context.renderSpace
			},
			Sprite{
				.color = Vec4{1,0.5,1,1},
				.position = {context.bottomright, context.renderDepth},
				.scale = {5,5},
				.drawMode = context.renderSpace
			},
			Sprite{
				.color = Vec4{1,0.5,1,1},
				.position = Vec3{context.topleft.x, context.bottomright.y, context.renderDepth},
				.scale = {5,5},
				.drawMode = context.renderSpace
			},
			Sprite{
				.color = Vec4{1,0.5,1,1},
				.position = Vec3{context.bottomright.x, context.topleft.y, context.renderDepth},
				.scale = {5,5},
				.drawMode = context.renderSpace
			}
		};
	}
}
