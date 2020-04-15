#pragma once

#include <vector>

#include "BaseTypes.h"
#include "RenderTypes.h"

namespace Physics {
	constexpr float maxMass = 1'000'000'000'000.f;
	constexpr float nullDelta = 0.00001f;
	constexpr float pushoutFactor = 1.01f;

	inline std::vector<Drawable> debugDrawables;
}


class CollidableAdapter {
public:
	CollidableAdapter(vec2 const& pos_, float const& rota_, vec2 const& vel_, vec2 const& size_, Form const& form_, bool dyn_) :
		position{ pos_ },
		rotation{ rota_ },
		velocity{ vel_ },
		size{ size_ },
		form{ form_ },
		dynamic{ dyn_ }
	{}


	inline vec2 const& getPos() const { return position; }
	inline float getRota() const { return rotation; }
	inline vec2 const& getVel() const { return velocity; }
	inline vec2 const& getSize() const { return size; }
	inline Form getForm() const { return form; }
	inline bool isDynamic() const { return dynamic; }

	inline float getRadius() const { return size.r * 0.5f; }
	inline float getSurfaceArea() const { return size.x * size.y; }
	inline vec2 getBoundsSize() const {
		if (form == Form::CIRCLE) {
			return vec2(getBoundsRadius() * 2);
		}
		else {
			return vec2(getBoundsRadius() * 2);
		}
	}

	inline float getBoundsRadius() const {
		if (form == Form::CIRCLE) {
			return size.r / 2.0f;
		}
		else {
			return sqrtf((size.x * size.x + size.y * size.y)) / 2.0f;
		}
	}
private:
	vec2  position;
	float rotation;
	vec2  velocity;
	vec2  size;
	Form  form;
	bool  dynamic;
};

template<typename T>
class Grid {
public:
	Grid() :
		minPos{ 0.0f, 0.0f },
		sizeX{ 0 },
		cellSize{ 0.1f, 0.1f }
	{}

	Grid(vec2 cellSize_) :
		minPos{ 0.0f, 0.0f },
		sizeX{ 0 },
		cellSize{ cellSize_ }
	{}

	inline T at(int x, int y) const {
		assert(x * sizeX + y < data.size());
		return data.at(x * sizeY + y);
	}

	inline void set(int x, int y, T val) {
		data.at(x * sizeY + y) = val;
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
	vec2 const getMinPos() const { return minPos; }
	vec2 const getCellSize() const { return cellSize; }

	vec2 minPos;
	vec2 cellSize;
private:
	int sizeX;
	int sizeY;
	std::vector<T> data;
};

struct CollisionTestResult {
	vec2 posChange;
	vec2 collisionNormal;
	vec2 collisionPos;
	float clippingDist;
	bool collided;

	CollisionTestResult() : posChange{ 0.0f }, collisionPos{ vec2(0,0) }, collided{ false }, clippingDist{ 0.0f }, collisionNormal{ 1,0 } {}
};

struct CollisionInfo {
	uint32_t idA;
	uint32_t idB;
	float clippingDist;
	vec2 collisionNormal;
	vec2 collisionPos;

	CollisionInfo(uint32_t idA_, uint32_t idB_, float clippingDist_, vec2 collisionNormal_, vec2 collisionPos_) :idA{ idA_ }, idB{ idB_ }, clippingDist{ clippingDist_ }, collisionNormal{ collisionNormal_ }, collisionPos{ collisionPos_ } {}
};

struct CollisionResponse {
	vec2 posChange;
};