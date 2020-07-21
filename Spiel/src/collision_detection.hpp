#pragma once

#include <array>

#include "BaseTypes.hpp"

#include "Vec2.hpp"

struct CollidableAdapter {
	CollidableAdapter(Vec2 const& pos_, float const& rota_, Vec2 const& size_, Form const& form_, bool dyn_, RotaVec2 rotationVec_) :
		position{ pos_ },
		size{ size_ },
		rotationVec{ rotationVec_ },
		form{ form_ },
		dynamic{ dyn_ }
	{}
	CollidableAdapter() :
		position{ 0,0 },
		size{ 0,0 },
		rotationVec{ 0,0 },
		form{ Form::Circle },
		dynamic{ false }
	{ }
	inline CollidableAdapter& operator=(CollidableAdapter const& rhs) = default;
	inline float getSurfaceArea() const { return size.x * size.y; }
	Vec2 position;
	Vec2 size;
	RotaVec2 rotationVec;
	Form form;
	bool dynamic;
};

struct CollisionTestResult {
	Vec2 collisionNormal;
	Vec2 collisionPos;
	float clippingDist;
	bool collided;

	CollisionTestResult() : collisionPos{ 0, 0 }, collided{ false }, clippingDist{ 0.0f }, collisionNormal{ 1,0 } {}
};

struct CollisionInfo {
	uint32_t idA;
	uint32_t idB;
	float clippingDist;
	Vec2 collisionNormal;
	Vec2 collisionPos;

	CollisionInfo(uint32_t idA_, uint32_t idB_, float clippingDist_, Vec2 collisionNormal_, Vec2 collisionPos_) :idA{ idA_ }, idB{ idB_ }, clippingDist{ clippingDist_ }, collisionNormal{ collisionNormal_ }, collisionPos{ collisionPos_ } {}
};

struct CollisionResponse {
	Vec2 posChange;
};

__forceinline float circleDist(Vec2 const pos1, float rad1, Vec2 const pos2, float rad2)
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

struct SATTestResult {
	bool didNotCollide{ false };
	float minClippingDist{ 1000000000000000.0f };
	Vec2 cornerPosOfMinClippingDist{ 0,0 };
	Vec2 collisionNormalOfMinClippingDist{ 1,0 };
};

SATTestResult partialSATTest(CollidableAdapter const& coll, CollidableAdapter const& other);

CollisionTestResult circleCircleCollisionCheck(CollidableAdapter const& coll, CollidableAdapter const& other, bool bothSolid);

CollisionTestResult rectangleRectangleCollisionCheck2(CollidableAdapter const& coll, CollidableAdapter const& other);

CollisionTestResult checkCircleRectangleCollision(CollidableAdapter const& circle, CollidableAdapter const& rect, bool bothSolid, bool isCirclePrimary);

inline bool isOverlappingAABB(Vec2 const a_pos, Vec2 const a_AABB, Vec2 const b_pos, Vec2 const b_AABB) {
	return (std::abs(b_pos.x - a_pos.x) < std::abs(b_AABB.x + a_AABB.x) * 0.5f)
		& (std::abs(b_pos.y - a_pos.y) < std::abs(b_AABB.y + a_AABB.y) * 0.5f);
}


CollisionTestResult collisionTest(CollidableAdapter const& coll_, CollidableAdapter const& other_);

CollisionTestResult collisionTestCachedAABB(CollidableAdapter const& coll_, CollidableAdapter const& other_, Vec2 aabbColl, Vec2 aabbOther);