#pragma once

#include <vector>

#include "Vec.hpp"
#include "EntityComponentStorage.hpp"
#include "EntityComponentManager.hpp"
#include "JobManager.hpp"

struct GridCell {
	GridCell(int maxWorkerCount) 
		:maxWorkerCount{ maxWorkerCount }
	{
		entitiesList.resize(maxWorkerCount);
	}

	void clear() {
		for (auto& list : entitiesList)
			list.clear();
	}

	std::vector<std::vector<Entity>> entitiesList;
	const int maxWorkerCount;
};

class CollisionGrid {
public:
	CollisionGrid(int xSize, int ySize, int maxWorkerCount)
		:xSize{ xSize }, ySize{ ySize } 
	{
		cells.resize(xSize* ySize, GridCell(maxWorkerCount));
	}

	GridCell& at(int x, int y) {
		return cells.at(x + y * xSize);
	}

	void clear() {
		for (auto& cell : cells)
			cell.clear();
	}

	void setSpace(Vec2 minPos, Vec2 maxPos) {

		this->minPos = minPos;
		this->maxPos = maxPos;

		cellSize = {
			(maxPos.x - minPos.x) / xSize,
			(maxPos.y - minPos.y) / ySize,
		};
	}

	struct GridInsertJob : public JobFunctor {
		EntityComponentManager& manager;
		CollisionGrid& collGrid;
		const std::vector<Entity>& entities;
		const std::vector<Vec2>& aabbs;
		const int firstIndex;

	public:
		GridInsertJob(EntityComponentManager& manager, CollisionGrid& collGrid, const std::vector<Entity>& entities, const std::vector<Vec2>& aabbs, int firstEnt)
			:manager{ manager }, collGrid{ collGrid }, entities{ entities }, aabbs{ aabbs }, firstIndex{ firstEnt } 
		{
		}

		void execute(int workerId) override {
			for (auto i = firstIndex; i < entities.size() && i < firstIndex + collGrid.entitiesPerJob; ++i) {
				insert(manager, entities.at(i),aabbs.at(entities.at(i)), workerId);
			}
		}

		void insert(EntityComponentManager& manager, const Entity entity, const Vec2 aabb, int workerId) {
			const Vec2 pos = manager.getComp<Base>(entity).position;

			auto [minX, minY] = collGrid.posToCell(pos - aabb * 0.5f);
			auto [maxX, maxY] = collGrid.posToCell(pos + aabb * 0.5f);

			for (auto x = minX; x <= maxX; ++x) {
				for (auto y = minY; y <= maxY; ++y) {
					collGrid.at(x, y).entitiesList.at(workerId).push_back(entity);
				}
			}
		}
	};

	void insert(JobManager& jobManager, EntityComponentManager& manager, const std::vector<Entity>& entities, const std::vector<Vec2>& aabbs) {
		std::vector<GridInsertJob> jobs;
		std::vector<Tag> tags;

		for (int i = 0; i < entities.size(); i += entitiesPerJob) {
			jobs.push_back(GridInsertJob(manager, *this, entities, aabbs, i));
		}
		for (auto& job : jobs) {
			tags.push_back(jobManager.addJob(&job));
		}

		jobManager.waitAndHelp(&tags);
	}

	std::pair<int, int> posToCell(Vec2 pos) {
		return {
			(int)((pos.x - minPos.x) / cellSize.x),
			(int)((pos.y - minPos.y) / cellSize.y)
		};
	}

	Vec2 cellToPos(int x, int y) {
		return {
			minPos.x + (x * cellSize.x),
			minPos.y + (y * cellSize.y)
		};
	}

	void querry(std::vector<Entity>& nearby, Vec2 pos, Vec2 aabb) {
		auto [minX, minY] = posToCell(pos - aabb * 0.5f);
		auto [maxX, maxY] = posToCell(pos + aabb * 0.5f);


		for (auto x = minX; x <= maxX; ++x) {
			for (auto y = minY; y <= maxY; ++y) {
				for (auto i = 0; i < at(x, y).entitiesList.size(); ++i) {
					nearby.insert(nearby.end(), at(x, y).entitiesList.at(i).begin(), at(x, y).entitiesList.at(i).end());
				}
			}
		}
	}

	//void debugQuerry(std::vector<Drawable>& debugDrawables, Vec2 pos, Vec2 aabb) {
	//	auto [minX, minY] = posToCell(pos - aabb * 0.5f);
	//	auto [maxX, maxY] = posToCell(pos + aabb * 0.5f);
	//
	//
	//	for (auto x = minX; x <= maxX; ++x) {
	//		for (auto y = minY; y <= maxY; ++y) {
	//			auto d = Drawable(0, cellToPos(x, y)+cellSize*0.5f, 0.4f, cellSize, Vec4(1, 0, 0, 1), Form::Rectangle,0);
	//			debugDrawables.push_back(d);
	//		}
	//	}
	//}

private:
	const int entitiesPerJob = 100;
	const int xSize;
	const int ySize;
	Vec2 cellSize{ 0, 0 };
	Vec2 minPos{ 0, 0 };
	Vec2 maxPos{ 0, 0 };
	std::vector<GridCell> cells;
};