#pragma once

#include <iostream>

#include <vector>
#include <array>

#include "glmath.h"
#include "Entity.h"

struct PosSize {
	PosSize(vec2 pos_, vec2 size_) :
		pos{pos_},
		size{size_} {}

	inline vec2 const& getPos() const { return pos; }
	inline vec2 const& getSize() const { return size; }

	vec2 pos;
	vec2 size;
};

class Quadtree {
public:
	Quadtree(float width, float hight, float xPos, float yPos, size_t capacity_)
		:size(vec2(width, hight)),
		pos(vec2(xPos, yPos)),
		collidables(),
		capacity{ capacity_ },
		hasSubTrees{ false },
		marked{ false }
	{}

	Quadtree(vec2 minPos_, vec2 maxPos_, size_t capacity_) :
		size(maxPos_ - minPos_),
		pos((maxPos_ - minPos_) / 2 + minPos_),
		capacity{ capacity_ },
		collidables(),
		hasSubTrees{ false },
		marked{ false }
	{}
public:

	void printContcoll(int i = 0) const;

	void insert(std::pair<uint32_t, PosSize> coll);

	void querry(std::vector<uint32_t> & rVec, vec2 pos, vec2 size) const;

	void querryWithDrawables(std::vector<uint32_t> & rVec, vec2 pos, vec2 size, std::vector<Drawable>& drawables) const;

	void clear();

	vec2 getPosition() { return pos; }
	vec2 getSize() { return size; }

private:

	inline std::tuple<bool, bool, bool, bool> isInSubtree(vec2 const& collPos, vec2 const& collSize) const {
		return std::tuple<bool, bool, bool, bool>(
			isOverlappingAABB2(collPos, collSize, ul->pos, ul->size),
			isOverlappingAABB2(collPos, collSize, ur->pos, ur->size),
			isOverlappingAABB2(collPos, collSize, dl->pos, dl->size),
			isOverlappingAABB2(collPos, collSize, dr->pos, dr->size));
	}

	inline bool isOverlappingAABB2(vec2 const& posA, vec2 const& sizeA, vec2 const& posB, vec2 const& sizeB) const {
		return fabs(posB.x - posA.x) <= fabs(sizeB.x + sizeA.x) * 0.5f &&
			fabs(posB.y - posA.y) <= fabs(sizeB.y + sizeA.y) * 0.5f;
	}

public:
	mutable bool marked;
private:
	bool hasSubTrees;
	// first = id, second.first = pos, second.second = size
	std::vector<std::pair<uint32_t, PosSize>> collidables;
	size_t capacity;
	vec2 size;
	vec2 pos;
	std::unique_ptr<Quadtree> ul;	//up left
	std::unique_ptr<Quadtree> ur;	//up right
	std::unique_ptr<Quadtree> dl;	//down left
	std::unique_ptr<Quadtree> dr;	//down right

};