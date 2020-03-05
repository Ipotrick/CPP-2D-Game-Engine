#pragma once

#include <iostream>

#include <vector>
#include <array>

#include "glmath.h"
#include "Entity.h"

class Quadtree {
public:
	Quadtree(uint32_t width, uint32_t hight, float xPos, float yPos, uint32_t capacity_)
		:size(vec2(width, hight)),
		pos(vec2(xPos, yPos)),
		collidables(),
		capacity{ capacity_ },
		hasSubTrees{ false },
		marked{ false }
	{
	}

	Quadtree(vec2 minPos_, vec2 maxPos_, uint32_t capacity_):
		size(maxPos_ - minPos_),
		pos((maxPos_ - minPos_)/2 + minPos_),
		capacity{ capacity_ },
		collidables(),
		hasSubTrees{ false },
		marked{ false }
	{}
public:

	void printContcoll(int i = 0) const;
	
	void insert(Collidable* coll);

	void querry(std::vector<Collidable*>& rVec, vec2 const& pos, vec2 const& size) const;

	void getDrawables(std::vector<Drawable>& rVec) const {
	}

	vec2 getPosition() { return pos; }
	vec2 getSize() { return size; }

private:

	inline std::tuple<bool, bool, bool, bool> isInSubtree(vec2 collPos, vec2 collSize) const {
		bool isInUl = isOverlappingAABB(collPos, collSize, ul->pos, ul->size);
		bool isInUr = isOverlappingAABB(collPos, collSize, ur->pos, ur->size);
		bool isInDl = isOverlappingAABB(collPos, collSize, dl->pos, dl->size);
		bool isInDr = isOverlappingAABB(collPos, collSize, dr->pos, dr->size);
		return std::tuple<bool, bool, bool, bool>(isInUl, isInUr, isInDl, isInDr);
	}

	inline bool isOverlappingAABB(vec2 const& posA, vec2 const& sizeA, vec2 const& posB, vec2 const& sizeB) const {
		return (posA.x + (sizeA.x * 0.5f)) > (posB.x - (sizeB.x * 0.5f))
			&& (posB.x + (sizeB.x * 0.5f)) > (posA.x - (sizeA.x * 0.5f))
			&& (posA.y + (sizeA.y * 0.5f)) > (posB.y - (sizeB.y * 0.5f))
			&& (posB.y + (sizeB.y * 0.5f)) > (posA.y - (sizeA.y * 0.5f));
	}

public:
	mutable bool marked;
private:
	bool hasSubTrees;
	std::vector<Collidable*> collidables;
	uint32_t capacity;
	vec2 size;
	vec2 pos;
	std::unique_ptr<Quadtree> ul;	//up left
	std::unique_ptr<Quadtree> ur;	//up right
	std::unique_ptr<Quadtree> dl;	//down left
	std::unique_ptr<Quadtree> dr;	//down right

};


//-----------------------------------------------------------


inline void Quadtree::printContcoll(int i) const {
	for (int j = 0; j < i; j++) std::cout << "  ";
	std::cout << "tree " << ": \n";
	for (auto& coll : collidables) {
		for (int j = 0; j < i; j++) std::cout << "  ";
		std::cout << "(" << coll->getPos().x << "," << coll->getPos().y << ")\n";
	}

	if (ul != nullptr) {
		for (int j = 0; j < i; j++) std::cout << "  ";
		std::cout << "ul\n";
		ul->printContcoll(i + 1);
	}
	if (ur != nullptr) {
		for (int j = 0; j < i; j++) std::cout << "  ";
		std::cout << "ur\n";
		ur->printContcoll(i + 1);
	}
	if (dl != nullptr) {
		for (int j = 0; j < i; j++) std::cout << "  ";
		std::cout << "dl\n";
		dl->printContcoll(i + 1);
	}
	if (dr != nullptr) {
		for (int j = 0; j < i; j++) std::cout << "  ";
		std::cout << "dr\n";
		dr->printContcoll(i + 1);
	}
}

inline void Quadtree::insert(Collidable* coll)
{
	if (hasSubTrees == false)
	{
		if (collidables.size() < capacity)
		{
			collidables.push_back(coll);
		}
		else
		{
			collidables.push_back(coll);
			auto collidablesOld = collidables;
			collidables.clear();

			hasSubTrees = true;
			ul = std::make_unique<Quadtree>(size.x * 0.5f, size.y * 0.5f,
				(pos.x - size.x * 0.25f),
				(pos.y - size.y * 0.25f),
				capacity);
			ul->collidables.reserve(capacity);
			ur = std::make_unique<Quadtree>(size.x * 0.5f, size.y * 0.5f,
				(pos.x + size.x * 0.25f),
				(pos.y - size.y * 0.25f),
				capacity);
			ur->collidables.reserve(capacity);
			dl = std::make_unique<Quadtree>(size.x * 0.5f, size.y * 0.5f,
				(pos.x - size.x * 0.25f),
				(pos.y + size.y * 0.25f),
				capacity);
			dl->collidables.reserve(capacity);
			dr = std::make_unique<Quadtree>(size.x * 0.5f, size.y * 0.5f,
				(pos.x + size.x * 0.25f),
				(pos.y + size.y * 0.25f),
				capacity);
			dr->collidables.reserve(capacity);
			for (auto& pcoll : collidablesOld)
			{
				auto [isInUl, isInUr, isInDl, isInDr] = isInSubtree(pcoll->getPos(), pcoll->getBoundsSize());
				int isInCount = (int)isInUl + (int)isInUr + (int)isInDl + (int)isInDr;
				if (isInCount > 1)
				{
					collidables.push_back(pcoll);
				}
				else
				{
					if (isInUl)
					{
						ul->insert(pcoll);
					}
					else if (isInUr)
					{
						ur->insert(pcoll);
					}
					else if (isInDl)
					{
						dl->insert(pcoll);
					}
					else if (isInDr)
					{
						dr->insert(pcoll);
					}
				}
			}
		}
	}
	else
	{
		auto [isInUl, isInUr, isInDl, isInDr] = isInSubtree(coll->getPos(), coll->getBoundsSize());
		int isInCount = (int)isInUl + (int)isInUr + (int)isInDl + (int)isInDr;
		if (isInCount > 1)
		{
			collidables.push_back(coll);
		}
		else
		{
			if (isInUl)
			{
				ul->insert(coll);
			}
			else if (isInUr)
			{
				ur->insert(coll);
			}
			else if (isInDl)
			{
				dl->insert(coll);
			}
			else if (isInDr)
			{
				dr->insert(coll);
			}
		}
	}
}

inline void Quadtree::querry(std::vector<Collidable*>& rVec, vec2 const& pos, vec2 const& size) const
{
	if (hasSubTrees == true)
	{
		auto [inUl, inUr, inDl, inDr] = isInSubtree(pos, size);

		if (inUl)
		{
			ul->querry(rVec, pos, size);
		}
		if (inUr)
		{
			ur->querry(rVec, pos, size);
		}
		if (inDl)
		{
			dl->querry(rVec, pos, size);
		}
		if (inDr)
		{
			dr->querry(rVec, pos, size);
		}
	}
	rVec.insert(rVec.end(), collidables.begin(), collidables.end());
}