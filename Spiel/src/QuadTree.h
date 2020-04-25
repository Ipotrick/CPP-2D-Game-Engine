#pragma once

#include <iostream>

#include <vector>
#include <array>

#include "BaseTypes.h"
#include "RenderTypes.h"

#include "World.h"



struct PosSize {
	PosSize(Vec2 pos_, Vec2 size_) :
		pos{pos_},
		size{size_} 
	{}

	inline Vec2 const& getPos() const { return pos; }
	inline Vec2 const& getSize() const { return size; }

	Vec2 pos;
	Vec2 size;
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
	Quadtree2(Vec2 minPos_, Vec2 maxPos_, size_t capacity_, World& wrld) :
		m_pos{ (maxPos_ - minPos_) / 2 + minPos_ },
		m_size{ maxPos_ - minPos_ },
		m_capacity{ capacity_ },
		nextFreeIndex{ 1 },
		world{ wrld }
	{
		trees.reserve(100);
		trees.push_back(QuadtreeNode());
	}

	void insert(uint32_t ent, uint32_t thisID, Vec2 thisPos, Vec2 thisSize);
	__forceinline void insert(uint32_t ent) {
		insert(ent, 0, m_pos, m_size);
	}
	void querry(std::vector<uint32_t>& rVec, PosSize const& posSize, uint32_t thisID, Vec2 thisPos, Vec2 thisSize) const;
	__forceinline void querry(std::vector<uint32_t>& rVec, PosSize const& posSize) const {
		querry(rVec, posSize, 0, m_pos, m_size);
	}
	
	void querryDebug(PosSize const& posSize, uint32_t thisID, Vec2 thisPos, Vec2 thisSize, std::vector<Drawable>& draw) const;
	__forceinline void querryDebug(PosSize const& posSize, std::vector<Drawable>& draw) const {
		querryDebug(posSize, 0, m_pos, m_size, draw);
	}
	void querryDebugAll(uint32_t thisID, Vec2 thisPos, Vec2 thisSize, std::vector<Drawable>& draw, Vec4 color, int depth) const;
	__forceinline void querryDebugAll(std::vector<Drawable>& draw, Vec4 color) const {
		querryDebugAll(0, m_pos, m_size, draw, color, 0);
	}

	void clear(uint32_t thisID);
	__forceinline void clear() {
		clear(0);
	}

	void resetPerPosSize(Vec2 pos, Vec2 size);

	void resetPerMinMax(Vec2 minPos, Vec2 maxPos);

	void removeEmptyLeafes(uint32_t thisID);
	__forceinline void removeEmptyLeafes() {
		removeEmptyLeafes(0);
	}

	__forceinline void setPosSize(Vec2 pos, Vec2 size) {
		m_pos = pos;
		m_size = size;
	}

	__forceinline Vec2 getPosition() const { return m_pos; }
	__forceinline Vec2 getSize() const { return m_size; }

private:

	inline std::tuple<bool, bool, bool, bool> isInSubtrees(Vec2 treePos, Vec2 treeSize, Vec2 pos, Vec2 size) const {
		return {
			isOverlappingAABB2(treePos + Vec2(-treeSize.x, -treeSize.y) * 0.25f, treeSize * 0.5f, pos, size),
			isOverlappingAABB2(treePos + Vec2( treeSize.x, -treeSize.y) * 0.25f, treeSize * 0.5f, pos, size),
			isOverlappingAABB2(treePos + Vec2(-treeSize.x,  treeSize.y) * 0.25f, treeSize * 0.5f, pos, size),
			isOverlappingAABB2(treePos + Vec2( treeSize.x,  treeSize.y) * 0.25f, treeSize * 0.5f, pos, size)
		};
	}

	__forceinline bool isOverlappingAABB2(Vec2 const& posA, Vec2 const& sizeA, Vec2 const& posB, Vec2 const& sizeB) const {
		return (fabs(posB.x - posA.x) <= fabs(sizeB.x + sizeA.x) * 0.5f) &
			(fabs(posB.y - posA.y) <= fabs(sizeB.y + sizeA.y) * 0.5f);
	}

	inline Vec2 boundsSize(Form form, Vec2 size, float rotation = 0.0f) {
		if (form == Form::CIRCLE) {
			return size;
		}
		else {
			Vec2 max{ 0,0 }; Vec2 min{ 0,0 };
			for (float i = -0.5f; i < 0.51f; i += 1.0f) {
				for (float j = -0.5f; j < 0.51f; j += 1.0f) {
					Vec2 point = rotate({ size.x * i, size.y * j }, rotation);
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
	Vec2 m_pos;
	Vec2 m_size;
	size_t m_capacity;
	std::vector<QuadtreeNode> trees;
	uint32_t nextFreeIndex;
	std::queue<uint32_t> freeIndices;
	World& world;
};

