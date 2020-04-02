#pragma once

#include <vector>
#include <array>

#include "glmath.h"
#include "Entity.h"

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
	inline vec2 const& getSize() const { return size;  }
	inline Form getForm() const { return form; }
	inline bool isDynamic() const { return dynamic; }

	inline float getRadius() const { return size.r * 0.5f; }
	inline float getSurfaceArea() const { return size.x * size.y; }
	inline vec2 getBoundsSize() const {
		if (form == Form::CIRCLE) {
			return vec2(getBoundsRadius() * 2);
		} else {
			return vec2(getBoundsRadius() * 2);
		}
	}

	inline float getBoundsRadius() const {
		if (form == Form::CIRCLE) {
			return size.r / 2.0f;
		} else {
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

	CollisionTestResult() : posChange{ 0.0f }, collisionPos{ vec2(0,0) }, collided{ false }, clippingDist{ 0.0f } {}
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

inline float calcMomentOfIntertia(float mass, vec2 size) {
	return mass * std::max(size.x, size.y) * std::max(size.x, size.y) / 12.0f * 2;
}

inline vec2 boundsSize(Form form, vec2 size, float rotation = 0.0f) {
	if (form == Form::CIRCLE) {
		return size;
	}
	else {
		vec2 max{ 0,0 }; vec2 min{ 0,0 };
		for (int i = -0.5f; i < 0.51f; i += 1.0f) {
			for (int j = -0.5f; j < 0.51f; j += 1.0f) {
				vec2 point = rotate({ size.x * i, size.y * j }, rotation);
				max.x = std::max(max.x, point.x);
				max.y = std::max(max.y, point.y);
				min.x = std::min(min.x, point.x);
				min.y = std::min(min.y, point.y);
			}
		}
		return max - min;
	}
}

__forceinline float dynamicCollision3(float v1, float m1, float v2, float m2, float e) {
	return (e * m2 * (v2 - v1) + m1 * v1 + m2 * v2) / (m1 + m2);
}

inline vec2 dynamicCollision2d3(vec2 const& v1, float const& m1, vec2 const& v2, float const& m2, vec2 const& cNV, float e) {
	float v1Coll = dot(v1, cNV);
	float v2Coll = dot(v2, cNV);
	if (v2Coll - v1Coll > 0.0f) {
		return (dynamicCollision3(v1Coll, m1, v2Coll, m2, e) - v1Coll) * cNV;
	}
	else {
		return 0;
	}
}

inline std::pair<std::pair<vec2, float>, std::pair< vec2, float>> dynamicCollision2d5(	vec2 const& posA, vec2 const& velA, float const anglVelA, float const massA, float const inertiaA, 
																						vec2 const& posB, vec2 const velB, float const anglVelB, float const massB, float const inertiaB, vec2 const& cNV, vec2 const& collPos, float e) {
	vec2 rAP = collPos - posA;
	vec2 rBP = collPos - posB;

	//speed the Collidables have at the specifiy collision point
	vec2 va = velA + rotate90(rAP) * anglVelA / RAD;
	vec2 vb = velB + rotate90(rAP) * anglVelB / RAD;
	float vAB_collDir = dot(va - vb, cNV);

	float j = (-(1.0f + e) * vAB_collDir) /
		(dot(cNV, (cNV * (1 / massA + 1 / massB))) + powf(cross(rAP, cNV), 2) / inertiaA + powf(cross(rBP, cNV), 2) / inertiaB);
	j = std::max(0.0f, j);	// j < 0 => they are not going into each other => no coll response
	return { {j / massA * cNV, cross(rAP, j * cNV) / inertiaA * RAD}, {-j / massB * cNV,-cross(rBP, j * cNV) / inertiaB * RAD} };
}

__forceinline float circleDist(vec2 const pos1, float rad1, vec2 const pos2, float rad2)
{
	float distCaer = distance(pos1, pos2);
	return distCaer - (rad1 + rad2);
}

__forceinline bool isIntervalCenterSmaller(float minFirst, float  maxFirst, float  minSecond, float  maxSecond)
{
	auto centerFirst = (maxFirst - minFirst) * 0.5f + minFirst;
	auto centerSecond = (maxSecond - minSecond) * 0.5f + minSecond;
	return (centerFirst < centerSecond);
}

__forceinline bool doIntervalsOverlap(float minFirst, float  maxFirst, float  minSecond, float  maxSecond)
{
	return !(minFirst > maxSecond || minSecond > maxFirst);
}

__forceinline float clippingDist(float minFirst, float  maxFirst, float  minSecond, float  maxSecond)
{
	if (minFirst > maxSecond || minSecond > maxFirst) {
		return 0.0f;
	}
	else {
		std::array<float, 2> distances{ fabs(minFirst - maxSecond), fabs(maxFirst - minSecond) };
		return *std::min_element(distances.begin(), distances.end());
	}
}

// primCollNormal is allways FROM other TO coll, dist is the dist TO push out, so dist > 0!
vec2 calcPosChange(CollidableAdapter const* coll, CollidableAdapter const* other, float const dist, vec2 const& primCollNormal);

CollisionTestResult circleCircleCollisionCheck(CollidableAdapter const* coll, CollidableAdapter const* other, bool bothSolid);

std::tuple<bool, float, float, vec2> partialSATCollision(CollidableAdapter const* coll, CollidableAdapter const* other);

CollisionTestResult rectangleRectangleCollisionCheck(CollidableAdapter const* coll, CollidableAdapter const* other, bool bothSolid);

CollisionTestResult checkCircleRectangleCollision(CollidableAdapter const* circle, CollidableAdapter const* rect, bool bothSolid, bool isCirclePrimary);

inline bool isOverlappingAABB(CollidableAdapter const* a, CollidableAdapter const* b) {
	return fabs(b->getPos().x - a->getPos().x) <= fabs(b->getBoundsSize().x + a->getBoundsSize().x) * 0.5f &&
	fabs(b->getPos().y - a->getPos().y) <= fabs(b->getBoundsSize().y + a->getBoundsSize().y) * 0.5f;
}

CollisionTestResult checkForCollision(CollidableAdapter const* coll_, CollidableAdapter const* other_, bool bothSolid);