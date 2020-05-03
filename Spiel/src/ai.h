#pragma once

#include <stack>
#include <limits>

#include "World.h"

#include "PhysicsTypes.h"

namespace ai {
	struct CellInfo {
		Vec2i parent{ -1, -1 };
		float g{ std::numeric_limits<float>::max() };
		float h{ std::numeric_limits<float>::max() };
		bool hasParent() { return parent.x != -1; }
	};

	std::stack<Vec2i> aStartSearch(Vec2i start, Vec2i end, Grid<bool> blockedGrid) {
		Grid<CellInfo> cells(blockedGrid.size());
		std::vector<Vec2i> frontier;
		frontier.push_back(start);

		auto isEnd = [&](Vec2i pos) { return pos == end; };

		auto bestCell = [&](std::vector<Vec2i> const& frontier, Grid<CellInfo> & grid) {
			auto best = frontier.begin();
			for (auto iter = std::next(best); iter != frontier.end(); ++iter) {
				if (grid.at(*iter).g + grid.at(*iter).h < grid.at(*best).g + grid.at(*best).h) {
					best = iter;
				}
			}
			return best;
		};

		auto canBeExpanded = [&](Vec2i pos) {
			return cells.isValid(pos) && !blockedGrid.at(pos);
		};

		auto h = [&](Vec2i pos) {
			return fabs(pos.x - end.x) + fabs(pos.y - end.y);
		};

		while (!frontier.empty()) {
			auto currentCell = bestCell(frontier, cells);
			frontier.erase(currentCell);
			blockedGrid.at(*currentCell) = true;

			// find best nearby cell:
			/*
					   north
						/|\
				west  <=   =>  east
						\|/
					   south
			*/
			bool foundNearbyCell{ false };
			Vec2i bestNearbyCell;
			// north:
			{
				Vec2i northCell = *currentCell + Vec2i(0, 1);
				if (canBeExpanded(northCell)) {
					if (!foundNearbyCell) {
						foundNearbyCell = true;
						bestNearbyCell = northCell;
					}
					else {
						if (h(northCell) < h(bestNearbyCell)) {
							bestNearbyCell = northCell;
						}
					}
				}
			}
			// east:
			{
				Vec2i eastCell = *currentCell + Vec2i(1, 0);
				if (canBeExpanded(eastCell)) {
					if (!foundNearbyCell) {
						foundNearbyCell = true;
						bestNearbyCell = eastCell;
					}
					else {
						if (h(eastCell) < h(bestNearbyCell)) {
							bestNearbyCell = eastCell;
						}
					}
				}
			}
			// south:
			{
				Vec2i southCell = *currentCell + Vec2i(0, -1);
				if (canBeExpanded(southCell)) {
					if (!foundNearbyCell) {
						foundNearbyCell = true;
						bestNearbyCell = southCell;
					}
					else {
						if (h(southCell) < h(bestNearbyCell)) {
							bestNearbyCell = southCell;
						}
					}
				}
			}
			// west:
			{
				Vec2i westCell = *currentCell + Vec2i(0, -1);
				if (canBeExpanded(westCell)) {
					if (!foundNearbyCell) {
						foundNearbyCell = true;
						bestNearbyCell = westCell;
					}
					else {
						if (h(westCell) < h(bestNearbyCell)) {
							bestNearbyCell = westCell;
						}
					}
				}
			}

			// save g, h and parent coords to expanded cell:

			if (!foundNearbyCell) {
				blockedGrid.at(*currentCell) = true;
			}
			else {
				// for the best cell expand the cell
				// calculate g and h
				cells.at(bestNearbyCell).h = h(bestNearbyCell);
				cells.at(bestNearbyCell).g = cells.at(*currentCell).g + 1;
				cells.at(bestNearbyCell).parent = *currentCell;
				frontier.push_back(bestNearbyCell);
			}
		}
	}
}