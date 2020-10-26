#include "QuadTree.hpp"

Quadtree3::Quadtree3(const Vec2 minPos_, const Vec2 maxPos_, const size_t capacity_, World& wrld, JobManager& jobManager, uint8_t TAG) :
	m_pos{ (maxPos_ - minPos_) / 2 + minPos_ },
	m_size{ maxPos_ - minPos_ },
	m_capacity{ capacity_ },
	world{ wrld },
	jobManager{ jobManager },
	IGNORE_TAG{ TAG }
{
	root.firstSubTree = nodes.make4Children();
	jobs.reserve(MAX_JOBS);
	tags.reserve(MAX_JOBS);
}



void Quadtree3::insert(const EntityHandleIndex ent, const std::vector<Vec2>& aabbs, const uint32_t thisID, const Vec2 thisPos, const Vec2 thisSize, int depth) 
{
	//printf("depth: %i\n", depth);
	auto& node = nodes.get(thisID);
	if (!node.hasSubTrees()) {
		if (node.collidables.size() < m_capacity || depth > MAX_DEPTH) {
			// if the node has no subtrees and is unter capacity, take the element into own storage:
			node.collidables.push_back(ent);
		}
		else {
			// if the node has no subtrees and is at capacity, split tree in subtrees:

			// clear collidables and save old collidables temporarily
			std::vector<EntityHandleIndex> collidablesOld;
			collidablesOld.reserve(node.collidables.size() + 1);
			collidablesOld = node.collidables;
			collidablesOld.push_back(ent);
			node.collidables.clear();
			node.firstSubTree = nodes.make4Children();

			// redistribute own collidables into subtrees:
			for (auto& pcoll : collidablesOld) {
				auto [isInUl, isInUr, isInDl, isInDr] = isInSubtrees(thisPos, thisSize, world.getComp<Transform>(pcoll).position,
					aabbs.at(pcoll));
				int isInCount = (int)isInUl + (int)isInDl + (int)isInUr + (int)isInDr;
				if (isInCount > STABILITY) {
					node.collidables.push_back(pcoll);
				}
				else {
					if (isInUl) {
						insert(pcoll, aabbs, node.firstSubTree + 0, thisPos + Vec2(-thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.5000001f, depth + 1);
					}
					if (isInUr) {
						insert(pcoll, aabbs, node.firstSubTree + 1, thisPos + Vec2(thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.5000001f, depth + 1);
					}
					if (isInDl) {
						insert(pcoll, aabbs, node.firstSubTree + 2, thisPos + Vec2(-thisSize.x, thisSize.y) * 0.25f, thisSize * 0.5000001f, depth + 1);
					}
					if (isInDr) {
						insert(pcoll, aabbs, node.firstSubTree + 3, thisPos + Vec2(thisSize.x, thisSize.y) * 0.25f, thisSize * 0.5000001f, depth + 1);
					}
				}
			}
		}
	}
	else {
		// this node has subtrees and tries to distribute them into the subtrees
		auto [isInUl, isInUr, isInDl, isInDr] = isInSubtrees(thisPos, thisSize, world.getComp<Transform>(ent).position,
			aabbs.at(ent));
		int isInCount = (int)isInUl + (int)isInDl + (int)isInUr + (int)isInDr;
		if (isInCount > STABILITY) {
			node.collidables.push_back(ent);
		}
		else {
			if (isInUl) {
				insert(ent, aabbs, node.firstSubTree + 0, thisPos + Vec2(-thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.5000001f, depth + 1);
			}
			if (isInUr) {
				insert(ent, aabbs, node.firstSubTree + 1, thisPos + Vec2(thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.5000001f, depth + 1);
			}
			if (isInDl) {
				insert(ent, aabbs, node.firstSubTree + 2, thisPos + Vec2(-thisSize.x, thisSize.y) * 0.25f, thisSize * 0.5000001f, depth + 1);
			}
			if (isInDr) {
				insert(ent, aabbs, node.firstSubTree + 3, thisPos + Vec2(thisSize.x, thisSize.y) * 0.25f, thisSize * 0.5000001f, depth + 1);
			}
		}
	}
}

void Quadtree3::insert(const EntityHandleIndex ent, const std::vector<Vec2>& aabbs)
{
	auto [isInUl, isInUr, isInDl, isInDr] = isInSubtrees(m_pos, m_size, world.getComp<Transform>(ent).position,
		aabbs.at(ent));
	int isInCount = (int)isInUl + (int)isInDl + (int)isInUr + (int)isInDr;
	if (isInCount > STABILITY) {
		root.collidables.push_back(ent);
	} 
	else {
		if (isInUl) {
			insert(ent, aabbs, root.firstSubTree + 0, m_pos + Vec2(-m_size.x, -m_size.y) * 0.25f, m_size * 0.5000001f, 1);
		}
		if (isInUr) {
			insert(ent, aabbs, root.firstSubTree + 1, m_pos + Vec2(m_size.x, -m_size.y) * 0.25f, m_size * 0.5000001f, 1);
		}
		if (isInDl) {
			insert(ent, aabbs, root.firstSubTree + 2, m_pos + Vec2(-m_size.x, m_size.y) * 0.25f, m_size * 0.5000001f, 1);
		}
		if (isInDr) {
			insert(ent, aabbs, root.firstSubTree + 3, m_pos + Vec2(m_size.x, m_size.y) * 0.25f, m_size * 0.5000001f, 1);
		}
	}
}

constexpr std::pair<float, float> iToFactor(int i)
{
	switch (i) {
	case 0:
		return { -1.0f, -1.0f };
	case 1:
		return { 1.0f, -1.0f };
	case 2:
		return { -1.0f,  1.0f };
	case 3:
		return { 1.0f,  1.0f };
	}
	return { 0.0f, 0.0f };
}

void Quadtree3::broadInsert(const std::vector<EntityHandleIndex>& entities, const std::vector<Vec2>& aabbs, const uint32_t thisID, const Vec2 thisPos, const Vec2 thisSize, const int depth)
{
	if (entities.empty()) return;
	auto& node = nodes.get(thisID);
	if (entities.size() <= m_capacity || depth > MAX_DEPTH) {
		node.collidables.insert(nodes.get(thisID).collidables.end(), entities.begin(), entities.end());
	}
	else {
		if (!node.hasSubTrees()) {
			node.firstSubTree = nodes.make4Children();
		}

		std::array<std::vector<EntityHandleIndex>*, 4> entityLists{
			new std::vector<EntityHandleIndex>(),
			new std::vector<EntityHandleIndex>(),
			new std::vector<EntityHandleIndex>(),
			new std::vector<EntityHandleIndex>()
		};
		auto& ul = *entityLists[0];
		auto& ur = *entityLists[1];
		auto& dl = *entityLists[2];
		auto& dr = *entityLists[3];
		ul.reserve(entities.size() / 3);
		ur.reserve(entities.size() / 3);
		dl.reserve(entities.size() / 3);
		dr.reserve(entities.size() / 3);
		for (auto ent : entities) {
			auto [isInUl, isInUr, isInDl, isInDr] = isInSubtrees(thisPos, thisSize, world.getComp<Transform>(ent).position, aabbs.at(ent));
			int isInCount = (int)isInUl + (int)isInDl + (int)isInUr + (int)isInDr;
			if (isInCount > STABILITY) {
				node.collidables.push_back(ent);
			}
			else {
				if (isInUl) ul.push_back(ent);
				if (isInUr) ur.push_back(ent);
				if (isInDl) dl.push_back(ent);
				if (isInDr) dr.push_back(ent);
			}
		}

		// for all four sub trees:
		for (auto i = 0; i < 4; i++) {
			const auto tuple = iToFactor(i);
			const auto xFactor = tuple.first;
			const auto yFactor = tuple.second;
		
			auto* entities = entityLists[i];
			auto* aabbsPtr = &aabbs;
		
			if (entityLists[i]->size() < MAX_ENTITIES_PER_JOB && jobs.size() < MAX_JOBS) {
				jobs.push_back(LambdaJob([this, aabbsPtr, thisID, entities, i, xFactor, yFactor, thisSize, thisPos, depth] (int workerId) 
					{
					auto node = nodes.get(thisID);
					for (const auto ent : *entities)
						insert(ent, *aabbsPtr, node.firstSubTree + i, thisPos + Vec2(thisSize.x * xFactor, thisSize.y * yFactor) * 0.25f, thisSize * 0.5000001f, depth + 1);
					delete entities;
					}));
				tags.push_back(jobManager.addJob(&jobs.back()));
			}
			else {
				broadInsert(*entityLists[i], aabbs, node.firstSubTree + i, thisPos + Vec2(thisSize.x * xFactor, thisSize.y * yFactor) * 0.25f, thisSize * 0.5000001f, depth + 1);
				delete entities;
			}
		}

		//// DO NOT DELETE, THIS CAN BE USED TO TEST IF THE MULTITHREADDING CONTAISN A BUG:
		//broadInsert(ul, aabbs, node.firstSubTree + 0, thisPos + Vec2(-thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.5000001f, depth + 1);
		//broadInsert(ur, aabbs, node.firstSubTree + 1, thisPos + Vec2(thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.5000001f, depth + 1);
		//broadInsert(dl, aabbs, node.firstSubTree + 2, thisPos + Vec2(-thisSize.x, thisSize.y) * 0.25f, thisSize * 0.5000001f, depth + 1);
		//broadInsert(dr, aabbs, node.firstSubTree + 3, thisPos + Vec2(thisSize.x, thisSize.y) * 0.25f, thisSize * 0.5000001f, depth + 1);
	}
}

void Quadtree3::broadInsert(const std::vector<EntityHandleIndex>& entities, const std::vector<Vec2>& aabbs)
{
	std::vector<EntityHandleIndex> ul, ur, dl, dr;
	ul.reserve(entities.size() / 3);
	ur.reserve(entities.size() / 3);
	dl.reserve(entities.size() / 3);
	dr.reserve(entities.size() / 3);
	for (auto ent : entities) {
		if (!world.getComp<Collider>(ent).isIgnoredBy(IGNORE_TAG)) {
			auto [isInUl, isInUr, isInDl, isInDr] = isInSubtrees(m_pos, m_size, world.getComp<Transform>(ent).position, aabbs.at(ent));
			if (isInUl) ul.push_back(ent);
			if (isInUr) ur.push_back(ent);
			if (isInDl) dl.push_back(ent);
			if (isInDr) dr.push_back(ent);
		}
	}

	broadInsert(ul, aabbs, 0, m_pos + Vec2(-m_size.x, -m_size.y) * 0.25f, m_size * 0.5000001f, 1);
	broadInsert(ur, aabbs, 1, m_pos + Vec2(m_size.x, -m_size.y) * 0.25f, m_size * 0.5000001f, 1);
	broadInsert(dl, aabbs, 2, m_pos + Vec2(-m_size.x, m_size.y) * 0.25f, m_size * 0.5000001f, 1);
	broadInsert(dr, aabbs, 3, m_pos + Vec2(m_size.x, m_size.y) * 0.25f, m_size * 0.5000001f, 1);

	jobManager.waitAndHelp(&tags);
	tags.clear();
	jobs.clear();
}

void Quadtree3::querry(std::vector<EntityHandleIndex>& rVec, const Vec2 qryPos, const Vec2 qrySize, const uint32_t thisID, const Vec2 thisPos, const  Vec2 thisSize) const 
{
	const auto& node = nodes.get(thisID);
	for (const auto ent : node.collidables) {
		volatile auto dummy = rVec;
		rVec.push_back(ent);
	}
	if (node.hasSubTrees()) {
		auto [isInUl, isInUr, isInDl, isInDr] = isInSubtrees(thisPos, thisSize, qryPos, qrySize);
		if (isInUl) {
			querry(rVec, qryPos, qrySize, node.firstSubTree + 0, thisPos + Vec2(-thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.5000001f);
		}
		if (isInUr) {
			querry(rVec, qryPos, qrySize, node.firstSubTree + 1, thisPos + Vec2(thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.5000001f);
		}
		if (isInDl) {
			querry(rVec, qryPos, qrySize, node.firstSubTree + 2, thisPos + Vec2(-thisSize.x, thisSize.y) * 0.25f, thisSize * 0.5000001f);
		}
		if (isInDr) {
			querry(rVec, qryPos, qrySize, node.firstSubTree + 3, thisPos + Vec2(thisSize.x, thisSize.y) * 0.25f, thisSize * 0.5000001f);
		}
	}
}

void Quadtree3::querry(std::vector<EntityHandleIndex>& rVec, const Vec2 qryPos, const Vec2 qrySize) const 
{
	for (const auto ent : root.collidables)
		rVec.push_back(ent);
	auto [isInUl, isInUr, isInDl, isInDr] = isInSubtrees(m_pos, m_size, qryPos, qrySize);
	if (isInUl) {
		querry(rVec, qryPos, qrySize, root.firstSubTree + 0, m_pos + Vec2(-m_size.x, -m_size.y) * 0.25f, m_size * 0.5000001f);
	}
	if (isInUr) {
		querry(rVec, qryPos, qrySize, root.firstSubTree + 1, m_pos + Vec2(m_size.x, -m_size.y) * 0.25f, m_size * 0.5000001f);
	}
	if (isInDl) {
		querry(rVec, qryPos, qrySize, root.firstSubTree + 2, m_pos + Vec2(-m_size.x, m_size.y) * 0.25f, m_size * 0.5000001f);
	}
	if (isInDr) {
		querry(rVec, qryPos, qrySize, root.firstSubTree + 3, m_pos + Vec2(m_size.x, m_size.y) * 0.25f, m_size * 0.5000001f);
	}
}

void Quadtree3::querryDebug(const Vec2 qryPos, const Vec2 qrySize, const uint32_t thisID, const Vec2 thisPos, const Vec2 thisSize, std::vector<Drawable>& draw, const int depth) const 
{
	auto& node = nodes.get(thisID);
	if (node.hasSubTrees())
	{
		auto [inUl, inUr, inDl, inDr] = isInSubtrees(thisPos, thisSize, qryPos, qrySize);

		if (inUl) {
			querryDebug(qryPos, qrySize, node.firstSubTree + 0, thisPos + Vec2(-thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.500001f, draw, depth + 1);
		}
		if (inUr) {
			querryDebug(qryPos, qrySize, node.firstSubTree + 1, thisPos + Vec2(thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.500001f, draw, depth + 1);
		}
		if (inDl) {
			querryDebug(qryPos, qrySize, node.firstSubTree + 2, thisPos + Vec2(-thisSize.x, thisSize.y) * 0.25f, thisSize * 0.500001f, draw, depth + 1);
		}
		if (inDr) {
			querryDebug(qryPos, qrySize, node.firstSubTree + 3, thisPos + Vec2(thisSize.x, thisSize.y) * 0.25f, thisSize * 0.500001f, draw, depth + 1);
		}
	}
	else {
		draw.push_back(Drawable(0, thisPos, 0.1f + 0.01f * depth, thisSize, Vec4(0, 0, 0, 1), Form::Rectangle, RotaVec2(0)));
		draw.push_back(Drawable(0, thisPos, 0.11f + 0.01f * depth, thisSize - Vec2(0.02f, 0.02f), Vec4(1, 1, 1, 1), Form::Rectangle, RotaVec2(0)));
	}
}

void Quadtree3::querryDebugAll(const uint32_t thisID, const Vec2 thisPos, const  Vec2 thisSize, std::vector<Drawable>& draw, const Vec4 color, const int depth) const 
{
	auto& node = nodes.get(thisID);
	if (node.hasSubTrees()) {
		querryDebugAll(node.firstSubTree + 0, thisPos + Vec2(-thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.5f, draw, color, depth + 1);
		querryDebugAll(node.firstSubTree + 1, thisPos + Vec2(thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.5f, draw, color, depth + 1);
		querryDebugAll(node.firstSubTree + 2, thisPos + Vec2(-thisSize.x, thisSize.y) * 0.25f, thisSize * 0.5f, draw, color, depth + 1);
		querryDebugAll(node.firstSubTree + 3, thisPos + Vec2(thisSize.x, thisSize.y) * 0.25f, thisSize * 0.5f, draw, color, depth + 1);
	}
	else {
		draw.push_back(Drawable(0, thisPos, 0.1f + 0.01f * depth, thisSize, Vec4(0, 0, 0, 1), Form::Rectangle, RotaVec2(0)));
		draw.push_back(Drawable(0, thisPos, 0.11f + 0.01f * depth, thisSize - Vec2(0.02f, 0.02f), color, Form::Rectangle, RotaVec2(0)));
	}
}

void Quadtree3::clear(const uint32_t thisID)
{
	nodes.get(thisID).collidables.clear();
	if (nodes.get(thisID).hasSubTrees()) {
		clear(nodes.get(thisID).firstSubTree + 0);
		clear(nodes.get(thisID).firstSubTree + 1);
		clear(nodes.get(thisID).firstSubTree + 2);
		clear(nodes.get(thisID).firstSubTree + 3);
	}
}

void Quadtree3::resetPerPosSize(const Vec2 pos, const Vec2 size)
{
	clear();
	m_pos = pos;
	m_size = size;
}

void Quadtree3::resetPerMinMax(const Vec2 minPos, const Vec2 maxPos)
{
	clear();
	Vec2 pos = 0.5f * (minPos + maxPos);
	Vec2 size = Vec2(fabs(maxPos.x - minPos.x), fabs(maxPos.y - minPos.y));
	m_pos = pos;
	m_size = size;
}

void Quadtree3::removeEmptyLeafes(const uint32_t thisID)
{
	if (nodes.get(thisID).hasSubTrees()) {
		for (int i = 0; i < 4; i++)
			removeEmptyLeafes(nodes.get(thisID).firstSubTree + i);
		if (!nodes.get(nodes.get(thisID).firstSubTree + 0).hasSubTrees()	// if no sub tree has sub trees
			&& !nodes.get(nodes.get(thisID).firstSubTree + 1).hasSubTrees()
			&& !nodes.get(nodes.get(thisID).firstSubTree + 2).hasSubTrees()
			&& !nodes.get(nodes.get(thisID).firstSubTree + 3).hasSubTrees())
		{
			if (nodes.get(nodes.get(thisID).firstSubTree + 0).collidables.empty()
				&& nodes.get(nodes.get(thisID).firstSubTree + 1).collidables.empty()
				&& nodes.get(nodes.get(thisID).firstSubTree + 2).collidables.empty()
				&& nodes.get(nodes.get(thisID).firstSubTree + 3).collidables.empty())
			{
				nodes.kill4Children(nodes.get(thisID).firstSubTree);
				nodes.get(thisID).firstSubTree = 0;
			}
		}
	}
}