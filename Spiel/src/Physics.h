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

	inline bool at(int x, int y) const {
		assert(x * sizeX + y < data.size());
		return data.at(x * sizeY + y);
	}

	inline void set(int x, int y, bool val = true) {
		data.at(x * sizeY + y) = val;
	}

	inline void clear() { data.clear(); }
	inline void resize(int x, int y) {
		sizeX = x;
		sizeY = y;
		data.resize(x * y);
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
	std::vector<bool> data;
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

__forceinline vec2 dynamicCollision2d3(vec2 const& v1, float const& m1, vec2 const& v2, float const& m2, vec2 const& cNV, float e) {
	float v1Coll = dot(v1, cNV);
	float v2Coll = dot(v2, cNV);
	if (v2Coll - v1Coll > 0.0f) {
		return (dynamicCollision3(v1Coll, m1, v2Coll, m2, e) - v1Coll) * cNV;
	}
	else {
		return 0;
	}
}

inline float fastAngle(vec2 v) {
	return atan2(dot(v, vec2(1, 0)), cross(v, vec2(1, 0)));
}

/*
__forceinline std::pair<std::pair<vec2, float>, std::pair< vec2, float>> dynamicCollision2d4(Collidable const& a, float const massA, float const inertiaA, Collidable b, float const massB, float const inertiaB, vec2 const& cNV, vec2 const& collPos, float e) {
	vec2 rAP = collPos - a.getPos();
	vec2 rBP = collPos - b.getPos();

	//speed the Collidables have at the specifiy collision point
	vec2 va = a.getVel() + rotate90(rAP) * a.getAnglVel() / RAD;
	vec2 vb = b.getVel() + rotate90(rAP) * b.getAnglVel() / RAD;
	float vAB_collDir = dot(va - vb, cNV);

	float j = (-(1.0f + e) * vAB_collDir) /
		(dot(cNV, (cNV * (1 / massA + 1 / massB))) + powf(cross(rAP, cNV), 2) / inertiaA + powf(cross(rBP, cNV), 2) / inertiaB);
	j = std::max(0.0f, j);	// j < 0 => they are not going into each other => no coll response
	return { {j / massA * cNV, cross(rAP, j * cNV) / inertiaA * RAD}, {-j / massB * cNV,-cross(rBP, j * cNV) / inertiaB * RAD} };
}*/

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
inline vec2 calcPosChange(CollidableAdapter const* coll, CollidableAdapter const* other, float const dist, vec2 const& primCollNormal) {
	if (other->isDynamic()) {
		float bothAreas = coll->getSurfaceArea() + other->getSurfaceArea();
		float bPart = other->getSurfaceArea() / bothAreas;
		float collDirV1 = dot(coll->getVel(), primCollNormal);
		float collDirV2 = dot(other->getVel(), primCollNormal);
		if (collDirV1 - collDirV2 > 0.0f && bPart < 0.75f && bPart > 0.25f) {
			//they move into each other
			if (collDirV2 < 0) {
				//coll moved into other
				return dist * primCollNormal * Physics::pushoutFactor;
			}
			else {
				//other moved into coll
			}
		}
		else {
			return dist * primCollNormal * bPart * Physics::pushoutFactor;
		}
	}
	else {
		return dist * primCollNormal * Physics::pushoutFactor;
	}
}

inline CollisionTestResult circleCircleCollisionCheck(CollidableAdapter const* coll, CollidableAdapter const* other, bool bothSolid)  {
	CollisionTestResult result = CollisionTestResult();
	float dist = circleDist(coll->getPos(), coll->getRadius(), other->getPos(), other->getRadius());

	if (dist < 0.0f)
	{
		result.collided = true;
		result.clippingDist = dist;
		result.collisionPos = normalize(coll->getPos() - other->getPos()) * other->getRadius() + other->getPos();

		auto collisionNormalVec = normalize(coll->getPos() - other->getPos());	/* a vector from b to coll_ */
		result.collisionNormal = collisionNormalVec;
		if (bothSolid) {
			result.posChange = calcPosChange(coll, other, -dist, collisionNormalVec);
		}
	}
	return result;
}

inline std::tuple<bool, float, float, vec2> partialSATCollision(CollidableAdapter const* coll, CollidableAdapter const* other)
{
	float resRotation = 0.0f;
	float minClippingDist = 10000000000000.0f;
	vec2 collisionPos{ 0,0 };
	for (int i = 0; i < 2; ++i)
	{
		float backRotationAngle = -coll->getRota() + 90 * i;
		auto backRotatedEntPos = rotate(coll->getPos(), backRotationAngle);
		float entHalfSize = (i == 0 ? coll->getSize().x * 0.5f : coll->getSize().y * 0.5f);
		float minEntProjPos = backRotatedEntPos.x - entHalfSize;
		float maxEntProjPos = backRotatedEntPos.x + entHalfSize;

		std::array<vec2, 4> otherCorners;
		otherCorners[0] = other->getPos() + rotate(vec2(other->getSize().x * 0.5f, other->getSize().y * 0.5f), other->getRota());
		otherCorners[1] = other->getPos() + rotate(vec2(other->getSize().x * 0.5f, -other->getSize().y * 0.5f), other->getRota());
		otherCorners[2] = other->getPos() + rotate(vec2(-other->getSize().x * 0.5f, other->getSize().y * 0.5f), other->getRota());
		otherCorners[3] = other->getPos() + rotate(vec2(-other->getSize().x * 0.5f, -other->getSize().y * 0.5f), other->getRota());
		for (int j = 0; j < 4; ++j) otherCorners[j] = rotate(otherCorners[j], backRotationAngle);

		auto comp = [](vec2 a, vec2 b) { return a.x < b.x; };
		vec2 minOEntProjPos = *std::min_element(otherCorners.begin(), otherCorners.end(), comp);
		vec2 maxOEntProjPos = *std::max_element(otherCorners.begin(), otherCorners.end(), comp);

		if (doIntervalsOverlap(minEntProjPos, maxEntProjPos, minOEntProjPos.x, maxOEntProjPos.x))
		{
			auto newClippingDist = clippingDist(minEntProjPos, maxEntProjPos, minOEntProjPos.x, maxOEntProjPos.x);
			if (newClippingDist < minClippingDist)
			{
				minClippingDist = newClippingDist;
				// set new clippingPos
				vec2 nearestCornerToCollCenter = otherCorners[0];
				for (int i = 1; i < 4; i++) {
					if (abs(otherCorners[i].x - rotate(coll->getPos(), backRotationAngle).x) < abs(nearestCornerToCollCenter.x - rotate(coll->getPos(), backRotationAngle).x)) {
						nearestCornerToCollCenter = otherCorners[i];
					}
				}
				collisionPos = rotate(nearestCornerToCollCenter, -backRotationAngle);

				resRotation = coll->getRota() + 270 * i;
				bool orientation = isIntervalCenterSmaller(minEntProjPos, maxEntProjPos, minOEntProjPos.x, maxOEntProjPos.x);
				resRotation += 180 * orientation;
			}
		}
		else
		{
			return std::tuple(false, 0.0f, 0.0f, vec2(0,0));
		}
	}
	resRotation = float((int)resRotation % 360);
	return std::tuple(true, resRotation, minClippingDist, collisionPos);
}

inline CollisionTestResult rectangleRectangleCollisionCheck(CollidableAdapter const* coll, CollidableAdapter const* other, bool bothSolid)
{
	CollisionTestResult result = CollisionTestResult();
	auto [partialResult1, rotation1, dist1, collPos1] = partialSATCollision(coll, other);
	if (partialResult1 == true) {
		auto [partialResult2, rotation2, dist2, collPos2] = partialSATCollision(other, coll);

		if (partialResult2)
		{
			result.collided = true;
			auto minClippingDist = dist1 < dist2 ? dist1 : dist2;
			result.clippingDist = minClippingDist;
			result.collisionPos = dist1 < dist2 ? collPos1 : collPos2;

			auto rotation = dist1 < dist2 ? rotation1 : rotation2 + 180;
			rotation = (float)((int)rotation % 360);

			auto collNormal = vec2(cosf(rotation / RAD), sinf(rotation / RAD));
			result.collisionNormal = collNormal;

			if (bothSolid) {
				result.posChange = calcPosChange(coll, other, minClippingDist, collNormal);
			}
		}
	}
	return result;
}

inline CollisionTestResult checkCircleRectangleCollision(CollidableAdapter const* circle, CollidableAdapter const* rect, bool bothSolid, bool isCirclePrimary) {
	CollisionTestResult result = CollisionTestResult();

	auto const& rotation = rect->getRota();
	auto rotCirclePos = rotate(circle->getPos(), -rotation);
	auto rotRectPos = rotate(rect->getPos(), -rotation);

	auto rectHalfSize = rect->getSize() * 0.5;

	/* calmp the circle position to the rectangle skin. This is the nbearest point of the circle to the rect. if circle is insided the rectangle, the clamping doesnt work. */
	auto clampedCirclePos = vec2(
		std::max(rotRectPos.x - rectHalfSize.x, std::min(rotCirclePos.x, rotRectPos.x + rectHalfSize.x)),
		std::max(rotRectPos.y - rectHalfSize.y, std::min(rotCirclePos.y, rotRectPos.y + rectHalfSize.y)) );

	/* check if circle center is inside the rectangle, as in this case the nearest point to a rectangle side has tto be computed differently */
	bool isCircleInsideRect{ false };
	if (clampedCirclePos.x < rotRectPos.x + rectHalfSize.x && clampedCirclePos.x > rotRectPos.x - rectHalfSize.x &&
		clampedCirclePos.y < rotRectPos.y + rectHalfSize.y && clampedCirclePos.y > rotRectPos.y - rectHalfSize.y) {
		/* now we need to find the closest side ot the rectangle relative to the circles center */
		auto differenceXLeft = fabs(rotCirclePos.x - (rotRectPos.x - rectHalfSize.x));
		auto differenceXRight = fabs(rotCirclePos.x - (rotRectPos.x + rectHalfSize.x));
		auto differenceYLeft = fabs(rotCirclePos.y - (rotRectPos.y - rectHalfSize.y));
		auto differenceYRight = fabs(rotCirclePos.y - (rotRectPos.y + rectHalfSize.y));
		if (differenceXLeft < differenceXRight && differenceXLeft < differenceYLeft && differenceXLeft < differenceYRight) {
			clampedCirclePos.x = rotRectPos.x - rectHalfSize.x;
		}
		else if (differenceXRight < differenceXLeft && differenceXRight < differenceYLeft && differenceXRight < differenceYRight) {
			clampedCirclePos.x = rotRectPos.x + rectHalfSize.x;
		}
		else if (differenceYLeft < differenceXLeft && differenceYLeft < differenceXRight && differenceYLeft < differenceYRight) {
			clampedCirclePos.y = rotRectPos.y - rectHalfSize.y;
		} 
		else {
			clampedCirclePos.y = rotRectPos.y + rectHalfSize.y;
		}
		isCircleInsideRect = true;
	}

	vec2 clampedToCircleCenter = rotCirclePos - clampedCirclePos;
	float clampedToCircleCenterLen = norm(clampedToCircleCenter);
	vec2 collisionDir;	//allways from rect TO circle
	float dist;
	if (isCircleInsideRect) {
		collisionDir = -normalize(clampedToCircleCenter);
		dist = -clampedToCircleCenterLen - circle->getRadius();
	}
	else {
		collisionDir = normalize(clampedToCircleCenter);
		dist = clampedToCircleCenterLen - circle->getRadius();
	}
	vec2 backRotatedCollNormal = rotate(collisionDir, rotation);

	if (dist < 0.0f) {
		result.collided = true;
		result.clippingDist = dist;
		result.collisionPos = rotate(clampedCirclePos, rotation);
		CollidableAdapter const* coll; CollidableAdapter const* other; vec2 primCollNormal;
		if (isCirclePrimary) {
			coll = circle; other = rect; primCollNormal = backRotatedCollNormal;
		}
		else {
			coll = rect; other = circle; primCollNormal = -backRotatedCollNormal;
		}
		result.collisionNormal = primCollNormal;

		if (bothSolid) {
			result.posChange = calcPosChange(coll, other, -dist, primCollNormal);
		}
		
	}
	return result;
}

inline bool isOverlappingAABB(CollidableAdapter const* a, CollidableAdapter const* b) {
	return fabs(b->getPos().x - a->getPos().x) <= fabs(b->getBoundsSize().x + a->getBoundsSize().x) * 0.5f &&
	fabs(b->getPos().y - a->getPos().y) <= fabs(b->getBoundsSize().y + a->getBoundsSize().y) * 0.5f;
}

inline CollisionTestResult checkForCollision(CollidableAdapter const* coll_, CollidableAdapter const* other_, bool bothSolid) {
	//pretest with AABB
	if (isOverlappingAABB(coll_, other_)) {
		if (coll_->getForm() == Form::CIRCLE) {
			if (other_->getForm() == Form::CIRCLE) {
				return circleCircleCollisionCheck(coll_, other_, bothSolid);
			}
			else {
				return checkCircleRectangleCollision(coll_, other_, bothSolid, true);
			}
		}
		else {
			if (other_->getForm() == Form::CIRCLE) {
				return checkCircleRectangleCollision(other_, coll_, bothSolid, false);
			}
			else {
				return rectangleRectangleCollisionCheck(coll_, other_, bothSolid);
			}
		}
	}
	return CollisionTestResult();
}