#pragma once

#include <vector>

#include "BaseTypes.hpp"
#include "RenderTypes.hpp"

namespace Physics {
	constexpr float maxMass = 1'000'000'000'000.f;
	constexpr float nullDelta = 0.00001f;
	constexpr float pushout = nullDelta;
	constexpr bool directionalPositionCorrection = true;
	constexpr float directionalCorrectionFactor = 0.75f;
	constexpr bool pressurebasedPositionCorrection = true;
	constexpr float pressureFalloff = 0.90f;

	inline std::vector<Drawable> debugDrawables;
}

template<typename T>
class GridPhysics {
public:
	GridPhysics() :
		minPos{ 0.0f, 0.0f },
		sizeX{ 0 },
		cellSize{ 0.1f, 0.1f }
	{}

	GridPhysics(Vec2 cellSize_) :
		minPos{ 0.0f, 0.0f },
		sizeX{ 0 },
		cellSize{ cellSize_ }
	{}

	inline T at(int x, int y) const {
		assert(x * sizeY + y < data.size());
		return data[x * sizeY + y];
	}

	inline void set(int x, int y, T val) {
		assert(x * sizeY + y < data.size());
		data[x * sizeY + y] = val;
	}

	inline void clear() { data.clear(); }
	inline void resize(int x, int y) {
		sizeX = x;
		sizeY = y;
		data.resize(x * y);
	}
	inline void resize(int x, int y, T val) {
		sizeX = x;
		sizeY = y;
		data.resize(x * y, val);
	}

	inline std::pair<int, int> getSize() const { return { sizeX, sizeY }; }
	inline int getSizeX() const { return sizeX; }
	inline int getSizeY() const { return sizeY; }
	Vec2 const getMinPos() const { return minPos; }
	Vec2 const getCellSize() const { return cellSize; }

	Vec2 minPos;
	Vec2 cellSize;
private:
	int sizeX;
	int sizeY;
	std::vector<T> data;
};