#pragma once
#include <iostream>
#include <vector>
#include <array>
#include <mutex>

#include "BaseTypes.hpp"
#include "RenderTypes.hpp"
#include "World.hpp"
#include "collision_detection.hpp"
#include "JobManager.hpp"

//struct QuadtreeNode {
//	QuadtreeNode() :
//		firstSubTree{ 0 }
//	{}
//
//	__forceinline bool hasSubTrees() const {
//		return (firstSubTree > 0);
//	}
//
//	std::vector<uint32_t> collidables;
//	uint32_t firstSubTree;
//};
//
//class Quadtree2 {
//	friend class QuadtreeNode;
//	const int stability = 2;	// 1 = high stability lower speed 2 = lower stability higher speed
//public:
//	Quadtree2(Vec2 minPos_, Vec2 maxPos_, size_t capacity_, World& wrld) :
//		m_pos{ (maxPos_ - minPos_) / 2 + minPos_ },
//		m_size{ maxPos_ - minPos_ },
//		m_capacity{ capacity_ },
//		nextFreeIndex{ 1 },
//		world{ wrld }
//	{
//		trees.reserve(100);
//		trees.push_back(QuadtreeNode());
//	}
//
//	void insert(Entity ent, std::vector<Vec2>& aabbs, uint32_t thisID, Vec2 thisPos, Vec2 thisSize, int depth = 0);
//	void insert(Entity ent, std::vector<Vec2>& aabbs) {
//		insert(ent, aabbs, 0, m_pos, m_size);
//	}
//	void querry(std::vector<Entity>& rVec, PosSize const& posSize, uint32_t thisID, Vec2 thisPos, Vec2 thisSize) const;
//	void querry(std::vector<Entity>& rVec, PosSize const& posSize) const {
//		querry(rVec, posSize, 0, m_pos, m_size);
//	}
//
//	void querryDebug(PosSize const& posSize, uint32_t thisID, Vec2 thisPos, Vec2 thisSize, std::vector<Drawable>& draw, int depth) const;
//	void querryDebug(PosSize const& posSize, std::vector<Drawable>& draw) const {
//		querryDebug(posSize, 0, m_pos, m_size, draw, 0);
//	}
//	void querryDebugAll(uint32_t thisID, Vec2 thisPos, Vec2 thisSize, std::vector<Drawable>& draw, Vec4 color, int depth) const;
//	void querryDebugAll(std::vector<Drawable>& draw, Vec4 color) const {
//		querryDebugAll(0, m_pos, m_size, draw, color, 0);
//	}
//
//	void clear(uint32_t thisID);
//	void clear() {
//		clear(0);
//	}
//
//	void resetPerPosSize(Vec2 pos, Vec2 size);
//
//	void resetPerMinMax(Vec2 minPos, Vec2 maxPos);
//
//	void removeEmptyLeafes(uint32_t thisID);
//	__forceinline void removeEmptyLeafes() {
//		removeEmptyLeafes(0);
//	}
//
//	__forceinline void setPosSize(Vec2 pos, Vec2 size) {
//		m_pos = pos;
//		m_size = size;
//	}
//
//	__forceinline Vec2 getPosition() const { return m_pos; }
//	__forceinline Vec2 getSize() const { return m_size; }
//
//private:
//
//	__forceinline std::tuple<bool, bool, bool, bool> isInSubtrees(Vec2 treePos, Vec2 treeSize, Vec2 pos, Vec2 size) const {
//		bool u = pos.y + size.y * 0.5f > treePos.y;
//		bool d = pos.y - size.y * 0.5f < treePos.y;
//		auto r = pos.x + size.x * 0.5f > treePos.x;
//		auto l = pos.x - size.x * 0.5f < treePos.x;
//		return {
//			d & l,
//			d & r,
//			u & l,
//			u & r
//		};
//	}
//public:
//	World& world;
//private:
//	Vec2 m_pos;
//	Vec2 m_size;
//	size_t m_capacity;
//	std::vector<QuadtreeNode> trees;
//	uint32_t nextFreeIndex;
//	std::queue<uint32_t> freeIndices;
//};