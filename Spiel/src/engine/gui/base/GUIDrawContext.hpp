#pragma once

#include "../../types/ShortNames.hpp"

#include "../../math/Vec.hpp"

namespace gui {
	struct Sizeing {
		enum class Mode {
			Relative,
			Absolute
		};

		Sizeing absX(float size) const
		{
			auto ret = *this;
			ret.xmode = Mode::Absolute;
			ret.x = size;
			return ret;
		}

		Sizeing absY(float size) const
		{
			auto ret = *this;
			ret.ymode = Mode::Absolute;
			ret.y = size;
			return ret;
		}

		Sizeing relX(float size) const
		{
			auto ret = *this;
			ret.xmode = Mode::Relative;
			ret.x = size;
			return ret;
		}

		Sizeing relY(float size) const
		{
			auto ret = *this;
			ret.ymode = Mode::Relative;
			ret.y = size;
			return ret;
		}

		Mode xmode{ Mode::Relative };
		float x{ 1.0f };
		Mode ymode{ Mode::Relative };
		float y{ 1.0f };
	};

	struct Placeing {
		enum class XMode {
			RelativeLeft,
			RelativeRight,
			AbsoluteLeft,
			AbsoluteRight
		};

		enum class YMode {
			RelativeTop,
			RelativeBottom,
			AbsoluteTop,
			AbsoluteBottom
		};

		Placeing absDistLeft(float dist) const
		{
			auto ret = *this;
			ret.xmode = XMode::AbsoluteLeft;
			ret.x = dist;
			return ret;
		}

		Placeing absDistRight(float dist) const
		{
			auto ret = *this;
			ret.xmode = XMode::AbsoluteRight;
			ret.x = dist;
			return ret;
		}

		Placeing absDistTop(float dist) const
		{
			auto ret = *this;
			ret.ymode = YMode::AbsoluteTop;
			ret.y = dist;
			return ret;
		}

		Placeing absDistBottom(float dist) const
		{
			auto ret = *this;
			ret.ymode = YMode::AbsoluteBottom;
			ret.y = dist;
			return ret;
		}

		Placeing relDistLeft(float dist) const
		{
			auto ret = *this;
			ret.xmode = XMode::RelativeLeft;
			ret.x = dist;
			return ret;
		}

		Placeing relDistRight(float dist) const
		{
			auto ret = *this;
			ret.xmode = XMode::AbsoluteRight;
			ret.x = dist;
			return ret;
		}

		Placeing relDistTop(float dist) const
		{
			auto ret = *this;
			ret.ymode = YMode::RelativeTop;
			ret.y = dist;
			return ret;
		}

		Placeing relDistBottom(float dist) const
		{
			auto ret = *this;
			ret.ymode = YMode::RelativeBottom;
			ret.y = dist;
			return ret;
		}

		XMode xmode{ XMode::RelativeLeft };
		float x{ 0.5 };
		YMode ymode{ YMode::RelativeTop };
		float y{ 0.5 };
	};

	enum class XAlign {
		/**
		 * elements are packed to the left.
		 */
		Left,
		/**
		 * elements are packed to the right.
		 */
		 Right,
		 /**
		 * elements are packed to the center.
		 */
		 Center
	};

	enum class YAlign {
		/**
		 * elements are packed to the top.
		 */
		Top,
		/**
		 * elements are packed to the bottom.
		 */
		Bottom,
		/**
		 * elements are packed to the center.
		 */
		Center
	};

	enum class Listing {
		Packed,
		Uniform
	};
	
	struct DrawContext {
		/**
		 * \return scaled size of the context.
		 */
		Vec2 size() const { return Vec2{ bottomright.x - topleft.x, topleft.y - bottomright.y }; }

		Vec2 centerpos() const { return 0.5f * (topleft + bottomright); }

		float left() const { return topleft.x; }

		float right() const { return bottomright.x; }

		float top() const { return topleft.y; }

		float bottom() const { return bottomright.y; }

		void cutLeft(float dist) { topleft.x += dist; }

		void cutRight(float dist) { bottomright.x -= dist; }

		void cutTop(float dist) { topleft.y -= dist; }

		void cutBottom(float dist) { bottomright.y += dist; }

		void assertState() const
		{
			assert(topleft.x <= bottomright.x && topleft.y >= bottomright.y);
		}
		XAlign xalign{ XAlign::Left };
		YAlign yalign{ YAlign::Top };
		Vec2 topleft{ 0.0f, 0.0f };
		Vec2 bottomright{ 0.0f, 0.0f };
		RenderSpace renderSpace{ RenderSpace::PixelSpace };
		float renderDepth{ 0.0f };
		float scale{ 1.0f };
		/**
		 * Tells flexible elements to either fill up all available space or to size themselfs the smallest size possible in the horizontal direction.
		 */
		bool bFlexFillX{ true };
		/**
		 * Tells flexible elements to either fill up all available space or to size themselfs the smallest size possible in vertical direction.
		 */
		bool bFlexFillY{ true };
	};

	struct Padding {
		Padding setTop(float p) const { Padding pad = *this; pad.top = p; return pad; }
		Padding setBottom(float p) const { Padding pad = *this; pad.bottom = p; return pad; }
		Padding setLeft(float p) const { Padding pad = *this; pad.left = p; return pad; }
		Padding setRight(float p) const { Padding pad = *this; pad.right = p; return pad; }
		Padding setX(float p) const { Padding pad = *this; pad.right = p; pad.left = p; return pad; }
		Padding setY(float p) const { Padding pad = *this; pad.top = p; pad.bottom = p; return pad; }

		float top{ 0.0f };
		float bottom{ 0.0f };
		float left{ 0.0f };
		float right{ 0.0f };
	};

	inline static DrawContext fit(DrawContext const& context, Vec2 scaledSize, Vec2 place)
	{
		DrawContext newcontext	= context;
		newcontext.topleft		= place + Vec2{ -scaledSize.x, scaledSize.y } * 0.5;
		newcontext.bottomright	= place - Vec2{ -scaledSize.x, scaledSize.y } * 0.5;
		return newcontext;
	}
}