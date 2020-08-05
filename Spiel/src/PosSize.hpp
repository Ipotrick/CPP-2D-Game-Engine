#pragma once

#include "Vec.hpp"

struct PosSize {
	PosSize(Vec2 pos_, Vec2 size_) :
		pos{ pos_ },
		size{ size_ }
	{}

	inline Vec2 const& getPos() const { return pos; }
	inline Vec2 const& getSize() const { return size; }

	Vec2 pos;
	Vec2 size;
};