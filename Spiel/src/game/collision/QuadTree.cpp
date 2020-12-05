#include "QuadTree.hpp"

Quadtree::Quadtree(const Vec2 minPos_, const Vec2 maxPos_, const size_t capacity_, CollisionSECM wrld, uint8_t TAG) :
	m_pos{ (maxPos_ - minPos_) / 2 + minPos_ },
	m_size{ maxPos_ - minPos_ },
	m_capacity{ capacity_ },
	world{ wrld },
	COLLIDER_TAG{ TAG }
{
	root.firstSubTree = nodes.make4Children();
}

void Quadtree::insert(const EntityHandleIndex ent, const std::vector<Vec2>& aabbs, const uint32_t thisID, const Vec2 thisPos, const Vec2 thisSize, int depth) 
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

void Quadtree::insert(const EntityHandleIndex ent, const std::vector<Vec2>& aabbs)
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

void Quadtree::broadInsert(
	std::vector<EntityHandleIndex>&& entities, 
	const std::vector<Vec2>& aabbs, 
	const uint32_t thisID, 
	const Vec2 thisPos, 
	const Vec2 thisSize, 
	const int depth)
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

		std::array entityLists{
			std::vector<EntityHandleIndex>(),
			std::vector<EntityHandleIndex>(),
			std::vector<EntityHandleIndex>(),
			std::vector<EntityHandleIndex>()
		};
		auto& ul = entityLists[0];
		auto& ur = entityLists[1];
		auto& dl = entityLists[2];
		auto& dr = entityLists[3];
		size_t thirdSize = entities.size() / 3;
		ul.reserve(thirdSize);
		ur.reserve(thirdSize);
		dl.reserve(thirdSize);
		dr.reserve(thirdSize);
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
		
			if (entityLists[i].size() < MAX_ENTITIES_PER_JOB && tags.size() < MAX_JOBS) {
				class InsertJob : public JobSystem::Job {
				public:
					InsertJob(
						Quadtree& qtree,
						std::vector<Vec2> const& aabbs,
						uint32_t thisID,
						std::vector<EntityHandleIndex>&& entities,
						int i,
						float xFactor,
						float yFactor,
						Vec2 thisSize,
						Vec2 thisPos,
						int depth)
					:
						qtree{ qtree },
						aabbs{ aabbs },
						thisID{ thisID },
						entities{ std::move(entities) },
						i{ i },
						xFactor{ xFactor },
						yFactor{ yFactor },
						thisSize{ thisSize },
						thisPos{ thisPos },
						depth{ depth }
					{ }

					void execute(uint32_t thread) override
					{
						auto& node = qtree.nodes.get(thisID);
						for (const auto ent : entities)
							qtree.insert(
								ent, 
								aabbs, 
								node.firstSubTree + i, 
								thisPos + Vec2(thisSize.x * xFactor, thisSize.y * yFactor) * 0.25f, 
								thisSize * 0.5000001f, 
								depth + 1
							);
					}

					Quadtree& qtree;
					std::vector<Vec2> const& aabbs;
					uint32_t thisID;
					std::vector<EntityHandleIndex> entities;
					int i;
					float xFactor;
					float yFactor;
					Vec2 thisSize;
					Vec2 thisPos;
					int depth;
				};

				auto tag = JobSystem::submit(InsertJob(
					*this,
					aabbs,
					thisID,
					std::move(entityLists[i]),
					i,
					xFactor,
					yFactor,
					thisSize,
					thisPos,
					depth
				));

				tags.push_back(tag);
			}
			else {
				broadInsert(std::move(entityLists[i]), aabbs, node.firstSubTree + i, thisPos + Vec2(thisSize.x * xFactor, thisSize.y * yFactor) * 0.25f, thisSize * 0.5000001f, depth + 1);
			}
		}

		//// DO NOT DELETE, THIS CAN BE USED TO TEST IF THE MULTITHREADDING CONTAISN A BUG:
		//broadInsert(ul, aabbs, node.firstSubTree + 0, thisPos + Vec2(-thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.5000001f, depth + 1);
		//broadInsert(ur, aabbs, node.firstSubTree + 1, thisPos + Vec2(thisSize.x, -thisSize.y) * 0.25f, thisSize * 0.5000001f, depth + 1);
		//broadInsert(dl, aabbs, node.firstSubTree + 2, thisPos + Vec2(-thisSize.x, thisSize.y) * 0.25f, thisSize * 0.5000001f, depth + 1);
		//broadInsert(dr, aabbs, node.firstSubTree + 3, thisPos + Vec2(thisSize.x, thisSize.y) * 0.25f, thisSize * 0.5000001f, depth + 1);
	}
}

void Quadtree::broadInsert(const std::vector<EntityHandleIndex>& entities, const std::vector<Vec2>& aabbs)
{
	std::vector<EntityHandleIndex> ul, ur, dl, dr;
	ul.reserve(entities.size() / 3);
	ur.reserve(entities.size() / 3);
	dl.reserve(entities.size() / 3);
	dr.reserve(entities.size() / 3);
	for (auto ent : entities) {
		if (!world.getComp<Collider>(ent).isIgnoredBy(COLLIDER_TAG)) {
			auto [isInUl, isInUr, isInDl, isInDr] = isInSubtrees(m_pos, m_size, world.getComp<Transform>(ent).position, aabbs.at(ent));
			if (isInUl) ul.push_back(ent);
			if (isInUr) ur.push_back(ent);
			if (isInDl) dl.push_back(ent);
			if (isInDr) dr.push_back(ent);
		}
	}

	broadInsert(std::move(ul), aabbs, 0, m_pos + Vec2(-m_size.x, -m_size.y) * 0.25f, m_size * 0.5000001f, 1);
	broadInsert(std::move(ur), aabbs, 1, m_pos + Vec2(m_size.x, -m_size.y) * 0.25f, m_size * 0.5000001f, 1);
	broadInsert(std::move(dl), aabbs, 2, m_pos + Vec2(-m_size.x, m_size.y) * 0.25f, m_size * 0.5000001f, 1);
	broadInsert(std::move(dr), aabbs, 3, m_pos + Vec2(m_size.x, m_size.y) * 0.25f, m_size * 0.5000001f, 1);

	for (auto tag : tags) {
		JobSystem::wait(tag);
	}
	tags.clear();
}


void Quadtree::querry(std::vector<EntityHandleIndex>& rVec, const Vec2 qryPos, const Vec2 qrySize, const uint32_t thisID, const Vec2 thisPos, const  Vec2 thisSize) const 
{
	const auto& node = nodes.get(thisID);
	for (const auto ent : node.collidables) {
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

void Quadtree::querry(std::vector<EntityHandleIndex>& rVec, std::vector<QtreeNodeQuerry>& frontier, const Vec2 qryPos, const Vec2 qrySize) const
{
	frontier.clear();
	if (frontier.capacity() < 20)
		frontier.reserve(20);
	
	for (const auto ent : root.collidables)
		rVec.push_back(ent);
	auto [isInUl, isInUr, isInDl, isInDr] = isInSubtrees(m_pos, m_size, qryPos, qrySize);
	if (isInUl) {
		frontier.push_back({ root.firstSubTree + 0, m_pos + Vec2(-m_size.x, -m_size.y) * 0.25f, m_size * 0.5000001f });
	}
	if (isInUr) {
		frontier.push_back({ root.firstSubTree + 1, m_pos + Vec2(m_size.x, -m_size.y) * 0.25f, m_size * 0.5000001f });
	}
	if (isInDl) {
		frontier.push_back({ root.firstSubTree + 2, m_pos + Vec2(-m_size.x, m_size.y) * 0.25f, m_size * 0.5000001f });
	}
	if (isInDr) {
		frontier.push_back({ root.firstSubTree + 3, m_pos + Vec2(m_size.x, m_size.y) * 0.25f, m_size * 0.5000001f });
	}
	
	while (!frontier.empty()) {
		QtreeNodeQuerry querry = frontier.back();
		frontier.pop_back();
		const auto& node = nodes.get(querry.nodeId);
		for (const auto ent : node.collidables) {
			rVec.push_back(ent);
		}
	
		if (node.hasSubTrees()) {
			auto [isInUl, isInUr, isInDl, isInDr] = isInSubtrees(querry.pos, querry.size, qryPos, qrySize);
			if (isInUl) {
				frontier.push_back({ node.firstSubTree + 0, querry.pos + Vec2(-querry.size.x, -querry.size.y) * 0.25f, querry.size * 0.5000001f });
			}
			if (isInUr) {
				frontier.push_back({ node.firstSubTree + 1, querry.pos + Vec2(querry.size.x, -querry.size.y) * 0.25f, querry.size * 0.5000001f });
			}
			if (isInDl) {
				frontier.push_back({ node.firstSubTree + 2, querry.pos + Vec2(-querry.size.x, querry.size.y) * 0.25f, querry.size * 0.5000001f });
			}
			if (isInDr) {
				frontier.push_back({ node.firstSubTree + 3, querry.pos + Vec2(querry.size.x, querry.size.y) * 0.25f, querry.size * 0.5000001f });
			}
		}
	}
	//for (const auto ent : root.collidables)
	//	rVec.push_back(ent);
	//auto [isInUl, isInUr, isInDl, isInDr] = isInSubtrees(m_pos, m_size, qryPos, qrySize);
	//if (isInUl) {
	//	querry(rVec, qryPos, qrySize, root.firstSubTree + 0, m_pos + Vec2(-m_size.x, -m_size.y) * 0.25f, m_size * 0.5000001f);
	//}
	//if (isInUr) {
	//	querry(rVec, qryPos, qrySize, root.firstSubTree + 1, m_pos + Vec2(m_size.x, -m_size.y) * 0.25f, m_size * 0.5000001f);
	//}
	//if (isInDl) {
	//	querry(rVec, qryPos, qrySize, root.firstSubTree + 2, m_pos + Vec2(-m_size.x, m_size.y) * 0.25f, m_size * 0.5000001f);
	//}
	//if (isInDr) {
	//	querry(rVec, qryPos, qrySize, root.firstSubTree + 3, m_pos + Vec2(m_size.x, m_size.y) * 0.25f, m_size * 0.5000001f);
	//}
}

void Quadtree::querryDebug(const Vec2 qryPos, const Vec2 qrySize, const uint32_t thisID, const Vec2 thisPos, const Vec2 thisSize, std::vector<Drawable>& draw, const int depth) const 
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

void Quadtree::querryDebugAll(const uint32_t thisID, const Vec2 thisPos, const  Vec2 thisSize, std::vector<Drawable>& draw, const Vec4 color, const int depth) const 
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

void Quadtree::clear(const uint32_t thisID)
{
	nodes.get(thisID).collidables.clear();
	if (nodes.get(thisID).hasSubTrees()) {
		clear(nodes.get(thisID).firstSubTree + 0);
		clear(nodes.get(thisID).firstSubTree + 1);
		clear(nodes.get(thisID).firstSubTree + 2);
		clear(nodes.get(thisID).firstSubTree + 3);
	}
}

void Quadtree::resetPerPosSize(const Vec2 pos, const Vec2 size)
{
	clear();
	m_pos = pos;
	m_size = size;
}

void Quadtree::resetPerMinMax(const Vec2 minPos, const Vec2 maxPos)
{
	clear();
	Vec2 pos = 0.5f * (minPos + maxPos);
	Vec2 size = Vec2(fabs(maxPos.x - minPos.x), fabs(maxPos.y - minPos.y));
	m_pos = pos;
	m_size = size;
}

void Quadtree::removeEmptyLeafes(const uint32_t thisID)
{
	auto& node = nodes.get(thisID);

	if (!node.hasSubTrees()) {
		return;
	}

	// a node is empty when it holds no children and has no elements stored
	bool allSubNodesEmpty{ true };
	for (int i = 0; i < 4; ++i) {
		auto subNodeID = node.firstSubTree + i;
		auto& subNode = nodes.get(subNodeID);

		if (subNode.hasSubTrees()) {
			removeEmptyLeafes(subNodeID);
			allSubNodesEmpty = false;
		}
		if (!subNode.collidables.empty()) {
			allSubNodesEmpty = false;
		}
	}

	if (allSubNodesEmpty) {
		nodes.kill4Children(node.firstSubTree);
		node.firstSubTree = QuadtreeNode::INVALID_ID;
	}
}