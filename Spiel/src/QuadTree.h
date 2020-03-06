#pragma once

#include <iostream>

#include <vector>
#include <array>

#include "glmath.h"
#include "Entity.h"

class Quadtree {
public:
	Quadtree(float width, float hight, float xPos, float yPos, uint32_t capacity_)
		:size(vec2(width, hight)),
		pos(vec2(xPos, yPos)),
		collidables(),
		capacity{ capacity_ },
		hasSubTrees{ false },
		marked{ false }
	{
		collidables.reserve(capacity_ + 1);
	}

	Quadtree(vec2 minPos_, vec2 maxPos_, uint32_t capacity_):
		size(maxPos_ - minPos_),
		pos((maxPos_ - minPos_)/2 + minPos_),
		capacity{ capacity_ },
		collidables(),
		hasSubTrees{ false },
		marked{ false }
	{
		collidables.reserve(capacity_ + 1);
	}
public:

	void printContcoll(int i = 0) const;
	
	void insert(Collidable* coll);

	void querry(std::vector<Collidable*>& rVec, vec2 const& pos, vec2 const& size) const;

	void querryWithDrawables(std::vector<Collidable*>& rVec, vec2 const& pos, vec2 const& size, std::vector<Drawable>& drawables) const;

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

			ur = std::make_unique<Quadtree>(size.x * 0.5f, size.y * 0.5f,
				(pos.x + size.x * 0.25f),
				(pos.y - size.y * 0.25f),
				capacity);
			dl = std::make_unique<Quadtree>(size.x * 0.5f, size.y * 0.5f,
				(pos.x - size.x * 0.25f),
				(pos.y + size.y * 0.25f),
				capacity);
			dr = std::make_unique<Quadtree>(size.x * 0.5f, size.y * 0.5f,
				(pos.x + size.x * 0.25f),
				(pos.y + size.y * 0.25f),
				capacity);

			for (auto& pcoll : collidablesOld)
			{
				auto [isInUl, isInUr, isInDl, isInDr] = isInSubtree(pcoll->getPos(), pcoll->getBoundsSize()*2);
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
#ifdef _DEBUG
					if (isInCount == 0) {
						std::cout << "FAILURE IN INSERTION" << std::endl;
					}
#endif
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

inline void Quadtree::querry(std::vector<Collidable*>& rVec, vec2 const& pos_, vec2 const& size_) const
{
	if (hasSubTrees == true)
	{
		auto [inUl, inUr, inDl, inDr] = isInSubtree(pos_, size_);

		if (inUl)
		{
			ul->querry(rVec, pos_, size_);
		}
		if (inUr)
		{
			ur->querry(rVec, pos_, size_);
		}
		if (inDl)
		{
			dl->querry(rVec, pos_, size_);
		}
		if (inDr)
		{
			dr->querry(rVec, pos_, size_);
		}
	}
	rVec.insert(rVec.end(), collidables.begin(), collidables.end());
}

inline void Quadtree::querryWithDrawables(std::vector<Collidable*>& rVec, vec2 const& pos_, vec2 const& size_, std::vector<Drawable>& drawables) const
{
	if (hasSubTrees == true)
	{
		auto [inUl, inUr, inDl, inDr] = isInSubtree(pos_, size_);

		if (inUl)
		{
			ul->querryWithDrawables(rVec, pos_, size_, drawables);
		}
		if (inUr)
		{
			ur->querryWithDrawables(rVec, pos_, size_, drawables);
		}
		if (inDl)
		{
			dl->querryWithDrawables(rVec, pos_, size_, drawables);
		}
		if (inDr)
		{
			dr->querryWithDrawables(rVec, pos_, size_, drawables);
		}
		if (!inUl && !inUr && !inDl && !inDr) {
			std::cout << "in no subtree! " << std::endl;
		}
	}
	else {
		drawables.push_back(Drawable(pos, 0.1, size, vec4(1, 1, 1, 1), 0));
	}
	rVec.insert(rVec.end(), collidables.begin(), collidables.end());
}