#pragma once

#include <iostream>

#include <vector>
#include <array>

#include "BaseTypes.h"
#include "RenderTypes.h"



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
/*
class Quadtree {
public:
	Quadtree(float width, float hight, float xPos, float yPos, size_t capacity_)
		:size(vec2(width, hight)),
		pos(vec2(xPos, yPos)),
		collidables(),
		capacity{ capacity_ },
		hasSubTrees{ false }
	{}

	Quadtree(vec2 minPos_, vec2 maxPos_, size_t capacity_) :
		size(maxPos_ - minPos_),
		pos((maxPos_ - minPos_) / 2 + minPos_),
		capacity{ capacity_ },
		collidables(),
		hasSubTrees{ false }
	{}

	void printContcoll(int i = 0) const;

	void insert(std::pair<uint32_t, PosSize> coll);

	void querry(std::vector<uint32_t> & rVec, PosSize const& posSize) const;

	void querryWithDrawables(std::vector<uint32_t> & rVec, vec2 pos, vec2 size, std::vector<Drawable>& drawables) const;

	void clear();

	vec2 getPosition() { return pos; }
	vec2 getSize() { return size; }

private:

	inline std::tuple<bool, bool, bool, bool> isInSubtree(vec2 const& collPos, vec2 const& collSize) const {
		return {
			isOverlappingAABB2(collPos, collSize, ul->pos, ul->size),
			isOverlappingAABB2(collPos, collSize, ur->pos, ur->size),
			isOverlappingAABB2(collPos, collSize, dl->pos, dl->size),
			isOverlappingAABB2(collPos, collSize, dr->pos, dr->size)
		};
	}

	inline bool isOverlappingAABB2(vec2 const& posA, vec2 const& sizeA, vec2 const& posB, vec2 const& sizeB) const {
		return fabs(posB.x - posA.x) <= fabs(sizeB.x + sizeA.x) * 0.5f &&
			fabs(posB.y - posA.y) <= fabs(sizeB.y + sizeA.y) * 0.5f;
	}

public:
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
};*/

struct QuadtreeNode {
	QuadtreeNode(float width, float height, float x, float y, uint16_t capacity_) :
		size{ width, height },
		pos{ x, y },
		capacity{ capacity_ },
		firstSubTree{ 0 }
	{ }

	inline bool hasSubTrees() const {
		return (firstSubTree > 0);
	}

	std::vector<std::pair<uint32_t, PosSize>> collidables;
	uint16_t capacity;
	vec2 size;
	vec2 pos;
	uint32_t firstSubTree;
};

class Quadtree2 {
	friend class QuadtreeNode;
public:
	Quadtree2(vec2 minPos_, vec2 maxPos_, uint16_t capacity_) :
		nextFreeIndex{ 1 }
	{
		size = maxPos_ - minPos_;
		pos = (maxPos_ - minPos_) / 2 + minPos_;
		trees.reserve(100);
		trees.emplace_back(size.x, size.y, pos.x, pos.y, capacity_);
	}

	void insert(std::pair<uint32_t, PosSize> coll, uint32_t thisID);
	inline void insert(std::pair<uint32_t, PosSize> coll) {
		insert(coll, 0);
	}
	void querry(std::vector<uint32_t>& rVec, PosSize const& posSize, uint32_t thisID) const;
	inline void querry(std::vector<uint32_t>& rVec, PosSize const& posSize) const {
		querry(rVec, posSize, 0);
	}
	void querryDebug(PosSize const& posSize, uint32_t thisID, std::vector<Drawable>& draw) const;
	void querryDebugAll(uint32_t thisID, std::vector<Drawable>& draw, vec4 color, int depth) const;
	inline void querryDebugAll(std::vector<Drawable>& draw, vec4 color) const {
		querryDebugAll(0, draw, color, 0);
	}

	inline vec2 getPosition() const { return pos; }
	inline vec2 getSize() const { return size; }

private:
	vec2 pos;
	vec2 size;

private:

	inline std::tuple<bool, bool, bool, bool> isInSubtrees(uint32_t const& firstNode, PosSize const& posSize) const {
		return {
			isOverlappingAABB2(posSize.pos, posSize.size, trees[firstNode].pos, trees[firstNode].size),
			isOverlappingAABB2(posSize.pos, posSize.size, trees[firstNode + 1].pos, trees[firstNode + 1].size),
			isOverlappingAABB2(posSize.pos, posSize.size, trees[firstNode + 2].pos, trees[firstNode + 2].size),
			isOverlappingAABB2(posSize.pos, posSize.size, trees[firstNode + 3].pos, trees[firstNode + 3].size)
		};
	}

	__forceinline bool isOverlappingAABB2(vec2 const& posA, vec2 const& sizeA, vec2 const& posB, vec2 const& sizeB) const {
		return fabs(posB.x - posA.x) <= fabs(sizeB.x + sizeA.x) * 0.5f &
			fabs(posB.y - posA.y) <= fabs(sizeB.y + sizeA.y) * 0.5f;
	}

private:

	std::vector<QuadtreeNode> trees;
	uint32_t nextFreeIndex;
};

inline void Quadtree2::insert(std::pair<uint32_t, PosSize> coll, uint32_t thisID)
{
	if (trees[thisID].collidables.size() == 0) trees[thisID].collidables.reserve(trees[thisID].capacity);
	if (!trees[thisID].hasSubTrees()) {
		if (trees[thisID].collidables.size() < trees[thisID].capacity) {
			// if the node has no subtrees and is unter capacity, take the element into own storage:
			trees[thisID].collidables.push_back(coll);
		}
		else {
			// if the node has no subtrees and is at capacity, split tree in subtrees:

			// clear collidables and save old collidables temporarily
			trees[thisID].collidables.push_back(coll);
			auto collidablesOld = trees[thisID].collidables;
			trees[thisID].collidables.clear();

			// make new Nodes:
			// ul (firstNode):
			trees.emplace_back(trees[thisID].size.x * 0.5f, trees[thisID].size.y * 0.5f, // generate ul tree
				(trees[thisID].pos.x - trees[thisID].size.x * 0.25f),
				(trees[thisID].pos.y - trees[thisID].size.y * 0.25f),
				trees[thisID].capacity);
			trees.emplace_back(trees[thisID].size.x * 0.5f, trees[thisID].size.y * 0.5f, // generate ur tree
				(trees[thisID].pos.x + trees[thisID].size.x * 0.25f),
				(trees[thisID].pos.y - trees[thisID].size.y * 0.25f),
				trees[thisID].capacity);
			trees.emplace_back(trees[thisID].size.x * 0.5f, trees[thisID].size.y * 0.5f, // generate dl tree
				(trees[thisID].pos.x - trees[thisID].size.x * 0.25f),
				(trees[thisID].pos.y + trees[thisID].size.y * 0.25f),
				trees[thisID].capacity);
			trees.emplace_back(trees[thisID].size.x * 0.5f, trees[thisID].size.y * 0.5f, // generate dr tree
				(trees[thisID].pos.x + trees[thisID].size.x * 0.25f),
				(trees[thisID].pos.y + trees[thisID].size.y * 0.25f),
				trees[thisID].capacity);
			trees[thisID].firstSubTree = nextFreeIndex;
			nextFreeIndex += 4;

			// redistribute own collidables into subtrees:
			for (auto& pcoll : collidablesOld)
			{
				auto [isInUl, isInUr, isInDl, isInDr] = isInSubtrees(trees[thisID].firstSubTree, pcoll.second);
				int isInCount = (int)isInUl + (int)isInUr + (int)isInDl + (int)isInDr;
				if (isInCount > 1)
				{
					trees[thisID].collidables.push_back(pcoll);
				}
				else
				{
					if (isInUl)
					{
						insert(pcoll, trees[thisID].firstSubTree);
					}
					else if (isInUr)
					{
						insert(pcoll, trees[thisID].firstSubTree+1);
					}
					else if (isInDl)
					{
						insert(pcoll, trees[thisID].firstSubTree+2);
					}
					else if (isInDr)
					{
						insert(pcoll, trees[thisID].firstSubTree+3);
					}
				}
			}
		}
	}
	else {
		// this node has subtrees and tries to distribute them into the subtrees
		auto [isInUl, isInUr, isInDl, isInDr] = isInSubtrees(trees[thisID].firstSubTree, coll.second);
		int isInCount = (int)isInUl + (int)isInUr + (int)isInDl + (int)isInDr;
		if (isInCount > 1)
		{
			trees[thisID].collidables.push_back(coll);
		}
		else
		{
			if (isInUl)
			{
				insert(coll, trees[thisID].firstSubTree);
			}
			else if (isInUr)
			{
				insert(coll, trees[thisID].firstSubTree+1);
			}
			else if (isInDl)
			{
				insert(coll, trees[thisID].firstSubTree+2);
			}
			else if (isInDr)
			{
				insert(coll, trees[thisID].firstSubTree+3);
			}
		}
	}
}

inline void Quadtree2::querry(std::vector<uint32_t>& rVec, PosSize const& posSize, uint32_t thisID) const {
	if (trees[thisID].hasSubTrees())
	{
		if (isOverlappingAABB2(posSize.pos, posSize.size, trees[trees[thisID].firstSubTree].pos, trees[trees[thisID].firstSubTree].size))
		{
			querry(rVec, posSize, trees[thisID].firstSubTree);
		}
		if (isOverlappingAABB2(posSize.pos, posSize.size, trees[trees[thisID].firstSubTree + 1].pos, trees[trees[thisID].firstSubTree + 1].size))
		{
			querry(rVec, posSize, trees[thisID].firstSubTree+1);
		}
		if (isOverlappingAABB2(posSize.pos, posSize.size, trees[trees[thisID].firstSubTree + 2].pos, trees[trees[thisID].firstSubTree + 2].size))
		{
			querry(rVec, posSize, trees[thisID].firstSubTree+2);
		}
		if (isOverlappingAABB2(posSize.pos, posSize.size, trees[trees[thisID].firstSubTree + 3].pos, trees[trees[thisID].firstSubTree + 3].size))
		{
			querry(rVec, posSize, trees[thisID].firstSubTree+3);
		}
	}
	for (auto coll : trees[thisID].collidables) {
		rVec.push_back(coll.first);
	}
}

inline void Quadtree2::querryDebug(PosSize const& posSize, uint32_t thisID, std::vector<Drawable>& draw) const {
	if (trees[thisID].hasSubTrees())
	{
		auto [inUl, inUr, inDl, inDr] = isInSubtrees(trees[thisID].firstSubTree, posSize);

		if (inUl)
		{
			querryDebug(posSize, trees[thisID].firstSubTree, draw);
		}
		if (inUr)
		{
			querryDebug(posSize, trees[thisID].firstSubTree + 1, draw);
		}
		if (inDl)
		{
			querryDebug( posSize, trees[thisID].firstSubTree + 2, draw);
		}
		if (inDr)
		{
			querryDebug(posSize, trees[thisID].firstSubTree + 3, draw);
		}
	}
	else {
		draw.push_back(Drawable(0, trees[thisID].pos, 0.2f, trees[thisID].size, vec4(1, 1, 1, 1), Form::RECTANGLE, 0));
	}
}

inline void Quadtree2::querryDebugAll(uint32_t thisID, std::vector<Drawable>& draw, vec4 color, int depth) const {
	if (trees[thisID].hasSubTrees())
	{
		querryDebugAll(trees[thisID].firstSubTree + 0, draw, color, depth+1);
		querryDebugAll(trees[thisID].firstSubTree + 1, draw, color, depth + 1);
		querryDebugAll(trees[thisID].firstSubTree + 2, draw, color, depth + 1);
		querryDebugAll(trees[thisID].firstSubTree + 3, draw, color, depth + 1);
	}
	else {
		draw.push_back(Drawable(0, trees[thisID].pos, 0.1f + 0.01f*depth, trees[thisID].size, vec4(0,0,0,1), Form::RECTANGLE, 0));
		draw.push_back(Drawable(0, trees[thisID].pos, 0.11f + 0.01f * depth, trees[thisID].size - vec2(0.02f,0.02f), color, Form::RECTANGLE, 0));
	}
}