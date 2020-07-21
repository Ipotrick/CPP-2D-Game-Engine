#include "QuadTree.h"

void Quadtree2::insert(uint32_t coll, uint32_t thisID, Vec2 thisPos, Vec2 thisSize)
{
	if (!trees[thisID].hasSubTrees()) {
		if (trees[thisID].collidables.size() < m_capacity) {
			// if the node has no subtrees and is unter capacity, take the element into own storage:
			trees[thisID].collidables.push_back(coll);
		}
		else {
			// if the node has no subtrees and is at capacity, split tree in subtrees:

			// clear collidables and save old collidables temporarily
			std::vector<entity_index_type> collidablesOld;
			collidablesOld.reserve(trees[thisID].collidables.size()+1);
			collidablesOld = trees[thisID].collidables;
			collidablesOld.push_back(coll);
			trees[thisID].collidables.clear();

			if (freeIndices.size() == 0) {
				trees.push_back(QuadtreeNode());
				trees.push_back(QuadtreeNode());
				trees.push_back(QuadtreeNode());
				trees.push_back(QuadtreeNode());
				trees[thisID].firstSubTree = nextFreeIndex;
				nextFreeIndex += 4;
			}
			else {
				trees[thisID].firstSubTree = freeIndices.front();
				freeIndices.pop();
			}

			// redistribute own collidables into subtrees:
			for (auto& pcoll : collidablesOld)
			{
				auto [isInUl, isInUr, isInDl, isInDr] = isInSubtrees(thisPos, thisSize, world.getComp<Base>(pcoll).position,
					aabbBounds(world.getComp<Collider>(pcoll).size, world.getComp<Base>(pcoll).rotaVec));
				int isInCount = (int)isInUl + (int)isInUr + (int)isInDl + (int)isInDr;
				if (isInCount > 1)
				{
					trees[thisID].collidables.push_back(pcoll);
				}
				else
				{
					if (isInUl)
					{
						insert(pcoll, trees[thisID].firstSubTree + 0, thisPos + Vec2(-thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.500001f);
					}
					else if (isInUr)
					{
						insert(pcoll, trees[thisID].firstSubTree + 1, thisPos + Vec2(thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.500001f);
					}
					else if (isInDl)
					{
						insert(pcoll, trees[thisID].firstSubTree + 2, thisPos + Vec2(-thisSize.x, thisSize.y) * 0.25f, thisSize * 0.500001f);
					}
					else if (isInDr)
					{
						insert(pcoll, trees[thisID].firstSubTree + 3, thisPos + Vec2(thisSize.x, thisSize.y) * 0.25f, thisSize * 0.500001f);
					}
					else {
						std::cerr << "FAILURE TO INSERT FOUND " << isInCount << " FITTING SUBTREES" << std::endl;
						std::cerr << "AABB: " << aabbBounds(world.getComp<Collider>(pcoll).size, world.getComp<Base>(pcoll).rotaVec) << " pos: " << world.getComp<Base>(pcoll).position << std::endl;
						std::cerr << "AABB MotherNode: " << thisSize << " Pos MotherNode: " << thisPos << std::endl;
						assert(false);
					}
				}
			}
		}
	}
	else {
		// this node has subtrees and tries to distribute them into the subtrees
		auto [isInUl, isInUr, isInDl, isInDr] = isInSubtrees(thisPos, thisSize, world.getComp<Base>(coll).position,
			aabbBounds(world.getComp<Collider>(coll).size, world.getComp<Base>(coll).rotaVec));
		int isInCount = (int)isInUl + (int)isInUr + (int)isInDl + (int)isInDr;
		if (isInCount > 1)
		{
			trees[thisID].collidables.push_back(coll);
		}
		else
		{
			if (isInUl)
			{
				insert(coll, trees[thisID].firstSubTree + 0, thisPos + Vec2(-thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.500001f);
			}
			else if (isInUr)
			{
				insert(coll, trees[thisID].firstSubTree + 1, thisPos + Vec2(thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.500001f);
			}
			else if (isInDl)
			{
				insert(coll, trees[thisID].firstSubTree + 2, thisPos + Vec2(-thisSize.x, thisSize.y) * 0.25f, thisSize * 0.500001f);
			}
			else if (isInDr)
			{
				insert(coll, trees[thisID].firstSubTree + 3, thisPos + Vec2(thisSize.x, thisSize.y) * 0.25f, thisSize * 0.500001f);
			}
			else {
				std::cerr << "FAILURE TO INSERT FOUND " << isInCount << " FITTING SUBTREES" << std::endl;
				std::cerr << "AABB: " << aabbBounds(world.getComp<Collider>(coll).size, world.getComp<Base>(coll).rotaVec) << " pos: " << world.getComp<Base>(coll).position << std::endl;
				std::cerr << "AABB MotherNode: " << thisSize << " Pos MotherNode: " << thisPos << std::endl;
				assert(false);
			}
		}
	}
}

void Quadtree2::querry(std::vector<uint32_t>& rVec, PosSize const& posSize, uint32_t thisID, Vec2 thisPos, Vec2 thisSize) const {
	if (trees[thisID].hasSubTrees())
	{
		auto [isInUl, isInUr, isInDl, isInDr] = isInSubtrees(thisPos, thisSize, posSize.pos, posSize.size);
		if (isInUl)
		{
			querry(rVec, posSize, trees[thisID].firstSubTree + 0, thisPos + Vec2(-thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.500001f );
		}
		if (isInUr)
		{
			querry(rVec, posSize, trees[thisID].firstSubTree + 1, thisPos + Vec2(thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.500001f);
		}
		if (isInDl)
		{
			querry(rVec, posSize, trees[thisID].firstSubTree + 2, thisPos + Vec2(-thisSize.x, thisSize.y) * 0.25f, thisSize * 0.500001f);
		}
		if (isInDr)
		{
			querry(rVec, posSize, trees[thisID].firstSubTree + 3, thisPos + Vec2(thisSize.x, thisSize.y) * 0.25f, thisSize * 0.500001f);
		}
	}
	rVec.reserve(rVec.size() + trees[thisID].collidables.size());
	rVec.insert(rVec.end(), trees[thisID].collidables.begin(), trees[thisID].collidables.end());
}

void Quadtree2::querryDebug(PosSize const& posSize, uint32_t thisID, Vec2 thisPos, Vec2 thisSize, std::vector<Drawable>& draw) const {
	if (trees[thisID].hasSubTrees())
	{
		auto [inUl, inUr, inDl, inDr] = isInSubtrees(thisPos, thisSize, posSize.pos, posSize.size);

		if (inUl)
		{
			querryDebug(posSize, trees[thisID].firstSubTree + 0, thisPos + Vec2(-thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.500001f, draw);
		}
		if (inUr)
		{
			querryDebug(posSize, trees[thisID].firstSubTree + 1, thisPos + Vec2( thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.500001f, draw);
		}
		if (inDl)
		{
			querryDebug(posSize, trees[thisID].firstSubTree + 2, thisPos + Vec2(-thisSize.x,  thisSize.y) * 0.25f, thisSize * 0.500001f, draw);
		}
		if (inDr)
		{
			querryDebug(posSize, trees[thisID].firstSubTree + 3, thisPos + Vec2( thisSize.x,  thisSize.y) * 0.25f, thisSize * 0.500001f, draw);
		}
	}
	else {
		draw.push_back(Drawable(0, thisPos, 0.2f, thisSize, Vec4(0, 0, 0, 1), Form::Rectangle, RotaVec2(0)));
	}
}

void Quadtree2::querryDebugAll(uint32_t thisID, Vec2 thisPos, Vec2 thisSize, std::vector<Drawable>& draw, Vec4 color, int depth) const {
	if (trees[thisID].hasSubTrees())
	{
		querryDebugAll(trees[thisID].firstSubTree + 0, thisPos + Vec2(-thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.5f,  draw, color, depth + 1);
		querryDebugAll(trees[thisID].firstSubTree + 1, thisPos + Vec2( thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.5f, draw, color, depth + 1);
		querryDebugAll(trees[thisID].firstSubTree + 2, thisPos + Vec2(-thisSize.x,  thisSize.y) * 0.25f, thisSize * 0.5f, draw, color, depth + 1);
		querryDebugAll(trees[thisID].firstSubTree + 3, thisPos + Vec2( thisSize.x,  thisSize.y) * 0.25f, thisSize * 0.5f, draw, color, depth + 1);
	}
	else {
		draw.push_back(Drawable(0, thisPos, 0.1f + 0.01f * depth, thisSize, Vec4(0, 0, 0, 1), Form::Rectangle, RotaVec2(0)));
		draw.push_back(Drawable(0, thisPos, 0.11f + 0.01f * depth, thisSize - Vec2(0.02f, 0.02f), color, Form::Rectangle, RotaVec2(0)));
	}
}

void Quadtree2::clear(uint32_t thisID)
{
	trees[thisID].collidables.clear();
	if (trees[thisID].hasSubTrees()) {
		clear(trees[thisID].firstSubTree + 0);
		clear(trees[thisID].firstSubTree + 1);
		clear(trees[thisID].firstSubTree + 2);
		clear(trees[thisID].firstSubTree + 3);
	}
}

void Quadtree2::resetPerPosSize(Vec2 pos, Vec2 size)
{
	clear();
	m_pos = pos;
	m_size = size;
}

void Quadtree2::resetPerMinMax(Vec2 minPos, Vec2 maxPos)
{
	clear();
	Vec2 pos = 0.5f * (minPos + maxPos);
	Vec2 size = Vec2(fabs(maxPos.x - minPos.x), fabs(maxPos.y - minPos.y));
	m_pos = pos;
	m_size = size;
}

void Quadtree2::removeEmptyLeafes(uint32_t thisID)
{
	if (trees[thisID].hasSubTrees()) {
		removeEmptyLeafes(trees[thisID].firstSubTree + 0);
		removeEmptyLeafes(trees[thisID].firstSubTree + 1);
		removeEmptyLeafes(trees[thisID].firstSubTree + 2);
		removeEmptyLeafes(trees[thisID].firstSubTree + 3);
		if (!trees[trees[thisID].firstSubTree + 0].hasSubTrees()	// if no sub tree has sub trees
			&& !trees[trees[thisID].firstSubTree + 1].hasSubTrees()
			&& !trees[trees[thisID].firstSubTree + 2].hasSubTrees()
			&& !trees[trees[thisID].firstSubTree + 3].hasSubTrees())
		{
			if (trees[trees[thisID].firstSubTree + 0].collidables.empty()
				&& trees[trees[thisID].firstSubTree + 1].collidables.empty()
				&& trees[trees[thisID].firstSubTree + 2].collidables.empty()
				&& trees[trees[thisID].firstSubTree + 3].collidables.empty())
			{
				freeIndices.push(trees[thisID].firstSubTree);
				trees[thisID].firstSubTree = 0;
			}
		}
	}
}
