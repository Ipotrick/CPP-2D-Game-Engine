#include "QuadTree.hpp"

void Quadtree2::insert(uint32_t ent, std::vector<Vec2>& aabbs, uint32_t thisID, Vec2 thisPos, Vec2 thisSize, int depth)
{

	if (!trees[thisID].hasSubTrees()) {
		if (trees[thisID].collidables.size() < m_capacity) {
			// if the node has no subtrees and is unter capacity, take the element into own storage:
			trees[thisID].collidables.push_back(ent);
		}
		else {
			// if the node has no subtrees and is at capacity, split tree in subtrees:

			// clear collidables and save old collidables temporarily
			std::vector<Entity> collidablesOld;
			collidablesOld.reserve(trees[thisID].collidables.size()+1);
			collidablesOld = trees[thisID].collidables;
			collidablesOld.push_back(ent);
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
					aabbs.at(pcoll) );
				int isInCount = (int)isInUl + (int)isInDl + (int)isInUr + (int)isInDr;
				if (isInCount > stability) {
					trees[thisID].collidables.push_back(pcoll);
				}
				else {
					if (isInUl)
					{
						insert(pcoll, aabbs, trees[thisID].firstSubTree + 0, thisPos + Vec2(-thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.5000001f, depth + 1);
					}
					if (isInUr)
					{
						insert(pcoll, aabbs, trees[thisID].firstSubTree + 1, thisPos + Vec2(thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.5000001f, depth + 1);
					}
					if (isInDl)
					{
						insert(pcoll, aabbs, trees[thisID].firstSubTree + 2, thisPos + Vec2(-thisSize.x, thisSize.y) * 0.25f, thisSize * 0.5000001f, depth + 1);
					}
					if (isInDr)
					{
						insert(pcoll, aabbs, trees[thisID].firstSubTree + 3, thisPos + Vec2(thisSize.x, thisSize.y) * 0.25f, thisSize * 0.5000001f, depth + 1);
					}
				}
			}
		}
	}
	else {
		// this node has subtrees and tries to distribute them into the subtrees
		auto [isInUl, isInUr, isInDl, isInDr] = isInSubtrees(thisPos, thisSize, world.getComp<Base>(ent).position,
			aabbs.at(ent));
		int isInCount = (int)isInUl + (int)isInDl + (int)isInUr + (int)isInDr;
		if (isInCount > stability) {
			trees[thisID].collidables.push_back(ent);
		}
		else {
			if (isInUl)
			{
				insert(ent, aabbs, trees[thisID].firstSubTree + 0, thisPos + Vec2(-thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.5000001f, depth + 1);
			}
			if (isInUr)
			{
				insert(ent, aabbs, trees[thisID].firstSubTree + 1, thisPos + Vec2(thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.5000001f, depth + 1);
			}
			if (isInDl)
			{
				insert(ent, aabbs, trees[thisID].firstSubTree + 2, thisPos + Vec2(-thisSize.x, thisSize.y) * 0.25f, thisSize * 0.5000001f, depth + 1);
			}
			if (isInDr)
			{
				insert(ent, aabbs, trees[thisID].firstSubTree + 3, thisPos + Vec2(thisSize.x, thisSize.y) * 0.25f, thisSize * 0.5000001f, depth + 1);
			}
		}
	}
}

void Quadtree2::querry(std::vector<uint32_t>& rVec, PosSize const& posSize, uint32_t thisID, Vec2 thisPos, Vec2 thisSize) const {
	rVec.insert(rVec.end(), trees[thisID].collidables.begin(), trees[thisID].collidables.end());
	if (trees[thisID].hasSubTrees())
	{
		auto [isInUl, isInUr, isInDl, isInDr] = isInSubtrees(thisPos, thisSize, posSize.pos, posSize.size);
		if (isInUl)
		{
			querry(rVec, posSize, trees[thisID].firstSubTree + 0, thisPos + Vec2(-thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.5000001f );
		}
		if (isInUr)
		{
			querry(rVec, posSize, trees[thisID].firstSubTree + 1, thisPos + Vec2(thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.5000001f);
		}
		if (isInDl)
		{
			querry(rVec, posSize, trees[thisID].firstSubTree + 2, thisPos + Vec2(-thisSize.x, thisSize.y) * 0.25f, thisSize * 0.5000001f);
		}
		if (isInDr)
		{
			querry(rVec, posSize, trees[thisID].firstSubTree + 3, thisPos + Vec2(thisSize.x, thisSize.y) * 0.25f, thisSize * 0.5000001f);
		}
	}
}

void Quadtree2::querryDebug(PosSize const& posSize, uint32_t thisID, Vec2 thisPos, Vec2 thisSize, std::vector<Drawable>& draw, int depth) const {
	if (trees[thisID].hasSubTrees())
	{
		auto [inUl, inUr, inDl, inDr] = isInSubtrees(thisPos, thisSize, posSize.pos, posSize.size);

		if (inUl)
		{
			querryDebug(posSize, trees[thisID].firstSubTree + 0, thisPos + Vec2(-thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.500001f, draw, depth + 1);
		}
		if (inUr)
		{
			querryDebug(posSize, trees[thisID].firstSubTree + 1, thisPos + Vec2( thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.500001f, draw, depth + 1);
		}
		if (inDl)
		{
			querryDebug(posSize, trees[thisID].firstSubTree + 2, thisPos + Vec2(-thisSize.x,  thisSize.y) * 0.25f, thisSize * 0.500001f, draw, depth + 1);
		}
		if (inDr)
		{
			querryDebug(posSize, trees[thisID].firstSubTree + 3, thisPos + Vec2( thisSize.x,  thisSize.y) * 0.25f, thisSize * 0.500001f, draw, depth + 1);
		}
	}
	else {
		draw.push_back(Drawable(0, thisPos, 0.1f + 0.01f * depth, thisSize, Vec4(0, 0, 0, 1), Form::Rectangle, RotaVec2(0)));
		draw.push_back(Drawable(0, thisPos, 0.11f + 0.01f * depth, thisSize - Vec2(0.02f, 0.02f), Vec4(1,1,1,1), Form::Rectangle, RotaVec2(0)));
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
