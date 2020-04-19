#include "QuadTree.h"


void Quadtree2::insert(uint32_t coll, uint32_t thisID, vec2 thisPos, vec2 thisSize)
{
	if (!trees[thisID].hasSubTrees()) {
		if (trees[thisID].collidables.size() < m_capacity) {
			// if the node has no subtrees and is unter capacity, take the element into own storage:
			trees[thisID].collidables.push_back(coll);
		}
		else {
			// if the node has no subtrees and is at capacity, split tree in subtrees:

			// clear collidables and save old collidables temporarily
			auto collidablesOld = trees[thisID].collidables;
			trees[thisID].collidables.clear();

			// make new Nodes:
			// ul (firstNode):
			trees.push_back(QuadtreeNode());
			trees.push_back(QuadtreeNode());
			trees.push_back(QuadtreeNode());
			trees.push_back(QuadtreeNode());
			trees[thisID].firstSubTree = nextFreeIndex;
			nextFreeIndex += 4;

			// redistribute own collidables into subtrees:
			for (auto& pcoll : collidablesOld)
			{
				auto [isInUl, isInUr, isInDl, isInDr] = isInSubtrees(thisPos, thisSize, world.getComp<Base>(pcoll).position,
					boundsSize(world.getComp<Collider>(pcoll).form, world.getComp<Collider>(pcoll).size, world.getComp<Base>(pcoll).rotation));
				int isInCount = (int)isInUl + (int)isInUr + (int)isInDl + (int)isInDr;
				if (isInCount > 1)
				{
					trees[thisID].collidables.push_back(pcoll);
				}
				else
				{
					if (isInUl)
					{
						insert(pcoll, trees[thisID].firstSubTree + 0, thisPos + vec2(-thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.5f);
					}
					else if (isInUr)
					{
						insert(pcoll, trees[thisID].firstSubTree + 1, thisPos + vec2( thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.5f);
					}
					else if (isInDl)
					{
						insert(pcoll, trees[thisID].firstSubTree + 2, thisPos + vec2(-thisSize.x,  thisSize.y) * 0.25f, thisSize * 0.5f);
					}
					else if (isInDr)
					{
						insert(pcoll, trees[thisID].firstSubTree + 3, thisPos + vec2( thisSize.x,  thisSize.y) * 0.25f, thisSize * 0.5f);
					}
				}
			}
			insert(coll, thisID, thisPos, thisSize);
		}
	}
	else {
		// this node has subtrees and tries to distribute them into the subtrees
		auto [isInUl, isInUr, isInDl, isInDr] = isInSubtrees(thisPos, thisSize, world.getComp<Base>(coll).position,
			boundsSize(world.getComp<Collider>(coll).form, world.getComp<Collider>(coll).size, world.getComp<Base>(coll).rotation));
		int isInCount = (int)isInUl + (int)isInUr + (int)isInDl + (int)isInDr;
		if (isInCount > 1)
		{
			trees[thisID].collidables.push_back(coll);
		}
		else
		{
			if (isInUl)
			{
				insert(coll, trees[thisID].firstSubTree + 0, thisPos + vec2(-thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.5f);
			}
			else if (isInUr)
			{
				insert(coll, trees[thisID].firstSubTree + 1, thisPos + vec2( thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.5f);
			}
			else if (isInDl)
			{
				insert(coll, trees[thisID].firstSubTree + 2, thisPos + vec2(-thisSize.x,  thisSize.y) * 0.25f, thisSize * 0.5f);
			}
			else if (isInDr)
			{
				insert(coll, trees[thisID].firstSubTree + 3, thisPos + vec2( thisSize.x,  thisSize.y) * 0.25f, thisSize * 0.5f);
			}
		}
	}
}

void Quadtree2::querry(std::vector<uint32_t>& rVec, PosSize const& posSize, uint32_t thisID, vec2 thisPos, vec2 thisSize) const {
	if (trees[thisID].hasSubTrees())
	{
		auto [isInUl, isInUr, isInDl, isInDr] = isInSubtrees(thisPos, thisSize, posSize.pos, posSize.size);
		if (isInUl)
		{
			querry(rVec, posSize, trees[thisID].firstSubTree + 0, thisPos + vec2(-thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.5f );
		}
		if (isInUr)
		{
			querry(rVec, posSize, trees[thisID].firstSubTree + 1, thisPos + vec2(thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.5f);
		}
		if (isInDl)
		{
			querry(rVec, posSize, trees[thisID].firstSubTree + 2, thisPos + vec2(-thisSize.x, thisSize.y) * 0.25f, thisSize * 0.5f);
		}
		if (isInDr)
		{
			querry(rVec, posSize, trees[thisID].firstSubTree + 3, thisPos + vec2(thisSize.x, thisSize.y) * 0.25f, thisSize * 0.5f);
		}
	}
	rVec.reserve(rVec.size() + trees[thisID].collidables.size());
	rVec.insert(rVec.end(), trees[thisID].collidables.begin(), trees[thisID].collidables.end());
}

void Quadtree2::querryDebug(PosSize const& posSize, uint32_t thisID, vec2 thisPos, vec2 thisSize, std::vector<Drawable>& draw) const {
	if (trees[thisID].hasSubTrees())
	{
		auto [inUl, inUr, inDl, inDr] = isInSubtrees(thisPos, thisSize, posSize.pos, posSize.size);

		if (inUl)
		{
			querryDebug(posSize, trees[thisID].firstSubTree + 0, thisPos + vec2(-thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.5f, draw);
		}
		if (inUr)
		{
			querryDebug(posSize, trees[thisID].firstSubTree + 1, thisPos + vec2( thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.5f, draw);
		}
		if (inDl)
		{
			querryDebug(posSize, trees[thisID].firstSubTree + 2, thisPos + vec2(-thisSize.x,  thisSize.y) * 0.25f, thisSize * 0.5f, draw);
		}
		if (inDr)
		{
			querryDebug(posSize, trees[thisID].firstSubTree + 3, thisPos + vec2( thisSize.x,  thisSize.y) * 0.25f, thisSize * 0.5f, draw);
		}
	}
	else {
		draw.push_back(Drawable(0, thisPos, 0.2f, thisSize, vec4(1, 1, 1, 1), Form::RECTANGLE, 0));
	}
}

void Quadtree2::querryDebugAll(uint32_t thisID, vec2 thisPos, vec2 thisSize, std::vector<Drawable>& draw, vec4 color, int depth) const {
	if (trees[thisID].hasSubTrees())
	{
		querryDebugAll(trees[thisID].firstSubTree + 0, thisPos + vec2(-thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.5f,  draw, color, depth + 1);
		querryDebugAll(trees[thisID].firstSubTree + 1, thisPos + vec2( thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.5f, draw, color, depth + 1);
		querryDebugAll(trees[thisID].firstSubTree + 2, thisPos + vec2(-thisSize.x,  thisSize.y) * 0.25f, thisSize * 0.5f, draw, color, depth + 1);
		querryDebugAll(trees[thisID].firstSubTree + 3, thisPos + vec2( thisSize.x,  thisSize.y) * 0.25f, thisSize * 0.5f, draw, color, depth + 1);
	}
	else {
		draw.push_back(Drawable(0, thisPos, 0.1f + 0.01f * depth, thisSize, vec4(0, 0, 0, 1), Form::RECTANGLE, 0));
		draw.push_back(Drawable(0, thisPos, 0.11f + 0.01f * depth, thisSize - vec2(0.02f, 0.02f), color, Form::RECTANGLE, 0));
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

void Quadtree2::resetPerPosSize(vec2 pos, vec2 size)
{
	clear();
	m_pos = pos;
	m_size = size;
}

void Quadtree2::resetPerMinMax(vec2 minPos, vec2 maxPos)
{
	clear();
	vec2 pos = 0.5f * (minPos + maxPos);
	vec2 size = vec2(fabs(maxPos.x - minPos.x), fabs(maxPos.y - minPos.y));
	m_pos = pos;
	m_size = size;
}
