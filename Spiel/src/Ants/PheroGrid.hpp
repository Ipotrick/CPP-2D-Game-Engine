#pragma once

#include <vector>

#include "../engine/types/ShortNames.hpp"
#include "../engine/math/vector_math.hpp"
#include "../engine/rendering/Sprite.hpp"

class PheroGrid {
public:
	PheroGrid(s32 cellsX, s32 cellsY, f32 cellSize):
		cellsX{ cellsX }, cellsY{ cellsY }, cellSize{ cellSize }
	{
		pheroStrength.resize((cellsX * 2 + 1) * (cellsY * 2 + 1), 0.0f);
		pheroStrengthCopy.resize((cellsX * 2 + 1)* (cellsY * 2 + 1), 0.0f);
		pheroSourceTimeDist.resize((cellsX * 2 + 1)* (cellsY * 2 + 1), std::numeric_limits<float>::max());
	}

	void plantAt(Vec2 pos, f32 strength, f32 sourceTimeDist)
	{
		auto [x, y] = getGridCoord(pos);
		auto& cellStr = strAt(x,y);
		auto& cellSrcDist = srcDistAt(x,y);
		cellSrcDist = std::min(cellSrcDist, sourceTimeDist);
		cellStr = std::min(MAX_CELL_OVERSATURATION, cellStr + strength);
	}

	f32& strAt(Vec2 pos)
	{
		u32 index = getGridIndex(pos);
		return strAt(index);
	}

	f32& strAt(u32 index)
	{
		return pheroStrength.at(index);
	}

	f32& strAt(s32 x, s32 y)
	{
		return pheroStrength.at((y + cellsY) * ((cellsX << 1)+1) + (x + cellsX));
	}

	f32& strAt(std::pair<s32,s32> coord)
	{
		return strAt(coord.first, coord.second);
	}

	f32& srcDistAt(u32 index)
	{
		return pheroSourceTimeDist.at(index);
	}

	f32& srcDistAt(s32 x, s32 y)
	{
		return pheroSourceTimeDist.at((y + cellsY) * ((cellsX << 1) + 1) + (x + cellsX));
	}

	std::pair<s32,s32> getGridCoord(const Vec2 pos)
	{
		const Vec2 relativePos = (pos + Vec2{1,1} * cellSize*0.5f - centerPos) / cellSize;
		const f32 x = std::clamp(s32(std::floorf(relativePos.x)), -cellsX, cellsX);
		const f32 y = std::clamp(s32(std::floorf(relativePos.y)), -cellsY, cellsY);
		return { x,y };
	}

	struct PheroCell { 
		f32 amount{ 0.0f };
		f32 srcDist{ 0.0f };
		Vec2 pos{ 0.0f, 0.0f };
	};
	std::array<PheroCell, 25> getNeighbors5(Vec2 pos)
	{
		auto [x, y] = getGridCoord(pos);
		x = std::clamp(x, -cellsX + 2, cellsX - 2);
		y = std::clamp(y, -cellsY + 2, cellsY - 2);
		const Vec2 centerCellPos = Vec2{ x * cellSize, y * cellSize } + centerPos;
		std::array<PheroCell, 25> res;
		u32 resi{ 0 };
		for (s32 xi = -2; xi <= 2; xi++) {
			for (s32 yi = -2; yi <= 2; yi++) {
				const u32 gridIndex = getGridIndex(x + xi, y + yi);
				res[resi].amount = pheroStrength[gridIndex];
				res[resi].srcDist = pheroSourceTimeDist[gridIndex];
				res[resi].pos = centerCellPos + Vec2{ xi * cellSize,yi * cellSize };
				resi++;
			}
		}
		return res;
	}

	u32 getGridIndex(s32 xc, s32 yc)
	{
		return (yc + cellsY) * ((cellsX<<1) + 1) + xc + cellsX;;
	}

	u32 getGridIndex(const Vec2 pos)
	{
		auto [x, y] = getGridCoord(pos);
		return getGridIndex(x, y);
	}

	void update(f32 dt)
	{
		std::vector<LambdaJob> fadingJobs;
		u32 begin{ 0 };
		u32 end{ 1000 };
		for (; begin < pheroStrength.size(); begin += 1000) {
			end = std::min(size_t(begin + 1000), pheroStrength.size());

			fadingJobs.push_back(LambdaJob(
				[begin, end, dt, this](u32 threadid) {
					for (u32 i = begin; i < end; i++) {
						auto& cell = this->pheroStrength[i];
						if (cell < 1.0f) {
							cell = std::clamp(cell - dt * this->strengthFade, 0.0f, MAX_CELL_OVERSATURATION);
						}
						else {
							cell = std::clamp(cell - dt * this->strengthFade * 10 * MAX_CELL_OVERSATURATION, 0.0f, MAX_CELL_OVERSATURATION);
						}

						const f32 pheroDistFalloff = std::clamp(MAX_CELL_OVERSATURATION - this->strengthFade, 1.0f, MAX_CELL_OVERSATURATION);
						this->pheroSourceTimeDist[i] += dt * this->srcDistFade * pheroDistFalloff;
					}
				}
			));
		}
		JobSystem::wait(JobSystem::submitVec(std::move(fadingJobs)));

		std::vector<LambdaJob> spreadJobs;
		for (s32 xc = -cellsX + 1; xc <= cellsX - 1; xc++) {
			auto jobFn = [this, xc, dt](u32 threadid) {
				for (s32 yc = -cellsY+1; yc <= cellsY-1; yc++) {
					f32 sum{ 0.0f };
				
					// pick the values of the 8 neighbors and oneself
					f32 cellNeighbors[9] = {
						strAt(xc - 1, yc - 1),
						strAt(xc + 0, yc - 1),
						strAt(xc + 1, yc - 1),
						strAt(xc - 1, yc + 0),
						strAt(xc + 0, yc + 0),
						strAt(xc + 1, yc + 0),
						strAt(xc - 1, yc + 1),
						strAt(xc + 0, yc + 1),
						strAt(xc + 1, yc + 1),
					};
				
					// make gaussian smoothed sum
					sum += 1.0f / 16.0f * cellNeighbors[0]; 
					sum += 2.0f / 16.0f * cellNeighbors[1];
					sum += 1.0f / 16.0f * cellNeighbors[2];
					sum += 2.0f / 16.0f * cellNeighbors[3];
					sum += 4.0f / 16.0f * cellNeighbors[4];
					sum += 2.0f / 16.0f * cellNeighbors[5];
					sum += 1.0f / 16.0f * cellNeighbors[6];
					sum += 2.0f / 16.0f * cellNeighbors[7];
					sum += 1.0f / 16.0f * cellNeighbors[8];
				
					u32 index = getGridIndex(xc, yc);
				
					pheroStrengthCopy[index] =
						dt* spread * (sum) +
						(1- dt * spread) * pheroStrength[index];
				}
			};

			spreadJobs.push_back(LambdaJob{ jobFn });
		}

		

		JobSystem::wait(JobSystem::submitVec(std::move(spreadJobs)));

		std::swap(pheroStrength, pheroStrengthCopy);
	}

	f32 expDecay(f32 x)
	{
		return 1.0f / (x + 1.0f);
	}

	std::vector<Sprite> draw(Vec4 color, bool timeOrIntensity)
	{
		std::vector<Sprite> res;
		for (s32 xc = -cellsX; xc <= cellsX; xc++) {
			for (s32 yc = -cellsY; yc <= cellsY; yc++) {
				const Vec2 pos = centerPos + Vec2{xc* cellSize,yc* cellSize };
				const Vec2 size = Vec2{ cellSize, cellSize };
				Vec4 c = color;
				if (!timeOrIntensity) {
					c *= (strAt(xc, yc) / MAX_CELL_OVERSATURATION);
				}
				else {
					c *= expDecay(srcDistAt(xc, yc)*0.1f);
				}
				c.a = color.a;
				if (strAt(xc, yc) > 0.01f) {
					res.push_back(
						Sprite{
							.color = c,
							.position = pos,
							.scale = size,
							.drawMode = RenderSpace::Camera,
						}
					);
				}
			}
		}
		return res;
	}

	f32 spread{ 0.001 };
	f32 strengthFade{ 0.004f };
	f32 srcDistFade{ 0.04f };
private:
	Vec2 centerPos{ 0,0 };
	s32 cellsX{ 1 };
	s32 cellsY{ 1 };
	f32 cellSize{ 1 };

	inline static const f32 MAX_CELL_OVERSATURATION = 4.0f;

	std::vector<f32> pheroStrength;
	std::vector<f32> pheroStrengthCopy;
	std::vector<f32> pheroSourceTimeDist;
};
