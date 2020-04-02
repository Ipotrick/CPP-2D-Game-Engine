#include "QuadTree.h"

void Quadtree::printContcoll(int i) const {
	for (int j = 0; j < i; j++) std::cout << "  ";
	std::cout << "tree " << ": \n";
	for (auto& coll : collidables) {
		for (int j = 0; j < i; j++) std::cout << "  ";
		std::cout << "(" << coll.second.pos.x << "," << coll.second.pos.y << ")\n";
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

void Quadtree::insert(std::pair<uint32_t, PosSize> coll)
{
	if (hasSubTrees == false)
	{
		if (collidables.capacity() == 0) {
			collidables.reserve(capacity);
		}
		if (collidables.size() < capacity)
		{
			collidables.emplace_back(coll);
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
				auto [isInUl, isInUr, isInDl, isInDr] = isInSubtree(pcoll.second.pos, pcoll.second.size);
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
		auto [isInUl, isInUr, isInDl, isInDr] = isInSubtree(coll.second.pos, coll.second.size);
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

void Quadtree::querry(std::vector<uint32_t>& rVec, vec2 pos, vec2 size) const
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
	rVec.reserve(rVec.size() + collidables.size());
	for (auto& coll : collidables) {
		rVec.push_back(coll.first);
	}
}

void Quadtree::querryWithDrawables(std::vector<uint32_t>& rVec, vec2 pos, vec2 size, std::vector<Drawable>& drawables) const
{
	if (hasSubTrees == true)
	{
		auto [inUl, inUr, inDl, inDr] = isInSubtree(pos, size);

		if (inUl)
		{
			ul->querryWithDrawables(rVec, pos, size, drawables);
		}
		if (inUr)
		{
			ur->querryWithDrawables(rVec, pos, size, drawables);
		}
		if (inDl)
		{
			dl->querryWithDrawables(rVec, pos, size, drawables);
		}
		if (inDr)
		{
			dr->querryWithDrawables(rVec, pos, size, drawables);
		}
		if (!inUl && !inUr && !inDl && !inDr) {
			std::cout << "in no subtree! " << std::endl;
		}
	}
	else {
		drawables.push_back(Drawable(0, pos, 0.1f, size, vec4(1, 1, 1, 1), Form::RECTANGLE, 0.0f));
	}
	rVec.reserve(rVec.size() + collidables.size());
	for (auto& coll : collidables) {
		rVec.push_back(coll.first);
	}
}

void Quadtree::clear() {
	collidables.clear();
	if (hasSubTrees) {
		ul->clear();
		ur->clear();
		dl->clear();
		dr->clear();
	}
}
