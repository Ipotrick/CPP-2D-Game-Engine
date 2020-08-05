#pragma once

#include <vector>

#include "robin_hood.h"

#include "JobManager.hpp"
#include "EntityComponentManager.hpp"
#include "EntityComponentStorage.hpp"
#include "Vec.hpp"
#include "PosSize.hpp"

class SAPBuffer {
	std::vector<std::pair<Entity, float>> maxYBounds;
	std::vector<int> maxYBoundsTable;
	std::vector<std::pair<Entity, float>> minYBounds;
	std::vector<int> minYBoundsTable;
	std::vector<std::pair<Entity, float>> minXBounds;
	std::vector<int> minXBoundsTable;
	std::vector<std::pair<Entity, float>> maxXBounds;
	std::vector<int> maxXBoundsTable;

	std::vector<robin_hood::unordered_set<Entity>> xSweepResult;

	Entity maxIndex = 0;
public:
	void clear() {
		maxYBounds.clear();
		maxYBoundsTable.clear();
		maxYBoundsTable.resize(maxIndex);
		minYBounds.clear();
		minYBoundsTable.clear();
		minYBoundsTable.resize(maxIndex);
		minXBounds.clear();
		minXBoundsTable.clear();
		minXBoundsTable.resize(maxIndex);
		maxXBounds.clear();
		maxXBoundsTable.clear();
		maxXBoundsTable.resize(maxIndex);
		xSweepResult.clear();
		xSweepResult.clear();
	}

	void setMaxEntityIndex(Entity maxIndex) {
		if (this->maxIndex < maxIndex) {
			maxYBoundsTable.resize(maxIndex, 0xFFFFFFFF); 
			minYBoundsTable.resize(maxIndex, 0xFFFFFFFF);
			minXBoundsTable.resize(maxIndex, 0xFFFFFFFF);
			maxXBoundsTable.resize(maxIndex, 0xFFFFFFFF); 
			xSweepResult.resize(maxIndex);
		}
		this->maxIndex = maxIndex;
	}

	void fillBounds(EntityComponentManager& world, std::vector<Vec2>& aabbCache) {
		for (auto ent : world.entity_view<Collider>()) {
			maxYBounds.push_back({ ent, world.getComp<Base>(ent).position.y + aabbCache.at(ent).y * 0.5f });
			minYBounds.push_back({ ent, world.getComp<Base>(ent).position.y - aabbCache.at(ent).y * 0.5f });
			minXBounds.push_back({ ent, world.getComp<Base>(ent).position.x - aabbCache.at(ent).x * 0.5f });
			maxXBounds.push_back({ ent, world.getComp<Base>(ent).position.x + aabbCache.at(ent).x * 0.5f });
		}
	}

	void sortBounds(JobManager& jobManager) {
		LambdaJob lambdaJobUper([&](int workerId) {
			std::sort(std::begin(maxYBounds), std::end(maxYBounds),
				[](std::pair<Entity, float> a, std::pair<Entity, float> b) {
					return a.second < b.second;
				});
			});
		auto tag1 = jobManager.addJob(&lambdaJobUper);

		LambdaJob lambdaJobLower([&](int workerId) {
			std::sort(std::begin(minYBounds), std::end(minYBounds),
				[](std::pair<Entity, float> a, std::pair<Entity, float> b) {
					return a.second < b.second;
				});
			});
		auto tag2 = jobManager.addJob(&lambdaJobLower
		);
		LambdaJob lambdaJobLeft([&](int workerId) {
			std::sort(std::begin(minXBounds), std::end(minXBounds),
				[](std::pair<Entity, float> a, std::pair<Entity, float> b) {
					return a.second < b.second;
				});
			});
		auto tag3 = jobManager.addJob(&lambdaJobLeft);

		LambdaJob lambdaJobRight([&](int workerId) {
			std::sort(std::begin(maxXBounds), std::end(maxXBounds),
				[](std::pair<Entity, float> a, std::pair<Entity, float> b) {
					return a.second < b.second;
				});
			});
		auto tag4 = jobManager.addJob(&lambdaJobRight);

		jobManager.waitFor(tag1);
		jobManager.waitFor(tag2);
		jobManager.waitFor(tag3);
		jobManager.waitFor(tag4);
	}

	void rebuildLookUpTables(JobManager& jobManager) {
		for (int i = 0; i < maxYBounds.size(); ++i) {
			maxYBoundsTable.at(maxYBounds.at(i).first) = i;
		}
		for (int i = 0; i < minYBounds.size(); ++i) {
			minYBoundsTable.at(minYBounds.at(i).first) = i;
		}
		for (int i = 0; i < minXBounds.size(); ++i) {
			minXBoundsTable.at(minXBounds.at(i).first) = i;
		}
		for (int i = 0; i < maxXBounds.size(); ++i) {
			maxXBoundsTable.at(maxXBounds.at(i).first) = i;
		}
	}

	Vec2 getXBounds(Entity ent) {
		return { minXBounds.at(minXBoundsTable.at(ent)).second ,maxXBounds.at(maxXBoundsTable.at(ent)).second };
	}

	void querry(std::vector<Entity>& out, Entity ent) {
		auto isIn = [](std::vector<Entity>& vec, Entity ent) {
			for (auto en : vec) {
				if (en == ent) return true;
			}
			return false;
		};
		// find potential overlaps horizontally:
		auto minX = minXBounds.at(minXBoundsTable.at(ent)).second;
		auto maxX = maxXBounds.at(maxXBoundsTable.at(ent)).second;

		int minBoundsIndex = minXBoundsTable.at(ent);
		for (int boundIndex = minXBoundsTable.at(ent) + 1; boundIndex < minXBounds.size() && minXBounds.at(boundIndex).second < maxX; boundIndex++) {
			auto candidate = minXBounds.at(boundIndex).first;
			if (!(minYBounds.at(minYBoundsTable.at(ent)).second > maxYBounds.at(maxYBoundsTable.at(candidate)).second
				|| minYBounds.at(minYBoundsTable.at(candidate)).second > maxYBounds.at(maxYBoundsTable.at(ent)).second)) {
				out.push_back(candidate);
			}
		}
		int maxBoundsIndex = maxXBoundsTable.at(ent);
		for (int boundIndex = maxXBoundsTable.at(ent) - 1; boundIndex >= 0 && maxXBounds.at(boundIndex).second > minX; boundIndex--) {
			auto candidate = minXBounds.at(boundIndex).first;
			if (!(minYBounds.at(minYBoundsTable.at(ent)).second > maxYBounds.at(maxYBoundsTable.at(candidate)).second
				|| minYBounds.at(minYBoundsTable.at(candidate)).second > maxYBounds.at(maxYBoundsTable.at(ent)).second)) {
				out.push_back(candidate);
			}
		}
	}
};