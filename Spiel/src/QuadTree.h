#pragma once

#include <iostream>

#include <vector>
#include <array>

#include "BaseTypes.h"
#include "RenderTypes.h"

#include "World.h"



struct PosSize {
	PosSize(vec2 pos_, vec2 size_) :
		pos{pos_},
		size{size_} 
	{}

	inline vec2 const& getPos() const { return pos; }
	inline vec2 const& getSize() const { return size; }

	vec2 pos;
	vec2 size;
};

struct QuadtreeNode {
	QuadtreeNode() :
		firstSubTree{ 0 }
	{}

	__forceinline bool hasSubTrees() const {
		return (firstSubTree > 0);
	}

	std::vector<uint32_t> collidables;
	uint32_t firstSubTree;
};

class Quadtree2 {
	friend class QuadtreeNode;
public:
	Quadtree2(vec2 minPos_, vec2 maxPos_, size_t capacity_, World& wrld) :
		m_pos{ (maxPos_ - minPos_) / 2 + minPos_ },
		m_size{ maxPos_ - minPos_ },
		m_capacity{ capacity_ },
		nextFreeIndex{ 1 },
		world{ wrld }
	{
		trees.reserve(100);
		trees.push_back(QuadtreeNode());
	}

	void insert(uint32_t ent, uint32_t thisID, vec2 thisPos, vec2 thisSize);
	__forceinline void insert(uint32_t ent) {
		insert(ent, 0, m_pos, m_size);
	}
	void querry(std::vector<uint32_t>& rVec, PosSize const& posSize, uint32_t thisID, vec2 thisPos, vec2 thisSize) const;
	__forceinline void querry(std::vector<uint32_t>& rVec, PosSize const& posSize) const {
		querry(rVec, posSize, 0, m_pos, m_size);
	}
	
	void querryDebug(PosSize const& posSize, uint32_t thisID, vec2 thisPos, vec2 thisSize, std::vector<Drawable>& draw) const;
	__forceinline void querryDebug(PosSize const& posSize, std::vector<Drawable>& draw) const {
		querryDebug(posSize, 0, m_pos, m_size, draw);
	}
	void querryDebugAll(uint32_t thisID, vec2 thisPos, vec2 thisSize, std::vector<Drawable>& draw, vec4 color, int depth) const;
	__forceinline void querryDebugAll(std::vector<Drawable>& draw, vec4 color) const {
		querryDebugAll(0, m_pos, m_size, draw, color, 0);
	}

	void clear(uint32_t thisID);
	__forceinline void clear() {
		clear(0);
	}

	void resetPerPosSize(vec2 pos, vec2 size);

	void resetPerMinMax(vec2 minPos, vec2 maxPos);

	__forceinline void setPosSize(vec2 pos, vec2 size) {
		m_pos = pos;
		m_size = size;
	}

	__forceinline vec2 getPosition() const { return m_pos; }
	__forceinline vec2 getSize() const { return m_size; }

private:

	inline std::tuple<bool, bool, bool, bool> isInSubtrees(vec2 treePos, vec2 treeSize, vec2 pos, vec2 size) const {
		return {
			isOverlappingAABB2(treePos + vec2(-treeSize.x, -treeSize.y) * 0.25f, treeSize * 0.5f, pos, size),
			isOverlappingAABB2(treePos + vec2( treeSize.x, -treeSize.y) * 0.25f, treeSize * 0.5f, pos, size),
			isOverlappingAABB2(treePos + vec2(-treeSize.x,  treeSize.y) * 0.25f, treeSize * 0.5f, pos, size),
			isOverlappingAABB2(treePos + vec2( treeSize.x,  treeSize.y) * 0.25f, treeSize * 0.5f, pos, size)
		};
	}

	__forceinline bool isOverlappingAABB2(vec2 const& posA, vec2 const& sizeA, vec2 const& posB, vec2 const& sizeB) const {
		return fabs(posB.x - posA.x) <= fabs(sizeB.x + sizeA.x) * 0.5f &
			fabs(posB.y - posA.y) <= fabs(sizeB.y + sizeA.y) * 0.5f;
	}

	inline vec2 boundsSize(Form form, vec2 size, float rotation = 0.0f) {
		if (form == Form::CIRCLE) {
			return size;
		}
		else {
			vec2 max{ 0,0 }; vec2 min{ 0,0 };
			for (float i = -0.5f; i < 0.51f; i += 1.0f) {
				for (float j = -0.5f; j < 0.51f; j += 1.0f) {
					vec2 point = rotate({ size.x * i, size.y * j }, rotation);
					max.x = std::max(max.x, point.x);
					max.y = std::max(max.y, point.y);
					min.x = std::min(min.x, point.x);
					min.y = std::min(min.y, point.y);
				}
			}
			return max - min;
		}
	}

private:
	vec2 m_pos;
	vec2 m_size;
	size_t m_capacity;
	std::vector<QuadtreeNode> trees;
	uint32_t nextFreeIndex;
	World& world;
};

