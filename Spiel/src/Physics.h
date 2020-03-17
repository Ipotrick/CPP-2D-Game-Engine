#pragma once

#include <vector>
#include <array>

#include "glmath.h"
#include "Entity.h"

namespace Physics {
	constexpr float maxMass = 1'000'000'000'000.f;
	constexpr float nullDelta = 0.00001f;
	constexpr float pushoutFactor = 1.05f;

	inline std::vector<Drawable> debugDrawables;

}

struct CollisionTestResult {
	vec2 posChange;
	vec2 collisionNormal;
	float clippingDist;
	bool collided;

	CollisionTestResult() : posChange{ 0.0f }, collided{ false }, clippingDist{ 0.0f } {}
};

struct CollisionInfo {
	uint32_t idA;
	uint32_t idB;
	float clippingDist;
	vec2 collisionNormal;

	CollisionInfo(uint32_t idA_, uint32_t idB_, float clippingDist_, vec2 collisionNormal_) :idA{ idA_ }, idB{ idB_ }, clippingDist{ clippingDist_ }, collisionNormal{ collisionNormal_ } {}
};

struct CollisionResponse {
	vec2 posChange;
};

__forceinline float dynamicCollision3(float v1, float m1, float v2, float m2, float e) {
	return (e*m2*(v2 - v1) + m1* v1 + m2* v2)/(m1 + m2);
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

inline CollisionTestResult circleCircleCollisionCheck(Collidable const* coll_, Collidable const* other_)  {
	CollisionTestResult result = CollisionTestResult();
	float dist = circleDist(coll_->getPos(), coll_->getRadius(), other_->getPos(), other_->getRadius());

	if (dist < 0.0f)
	{
		result.collided = true;
		result.clippingDist = dist;
		if (coll_->isSolid() && other_->isSolid()) {
			auto collisionNormalVec = normalize(coll_->getPos() - other_->getPos());	/* a vector from b to coll_ */ 
			result.collisionNormal = collisionNormalVec;
			auto circleOverlap = -dist;
			auto distVec = collisionNormalVec * circleOverlap;

			//float elasticity = std::min(std::max(coll_->getElasticity(), other_->getElasticity()), 1.0f);
			//auto velChange = dynamicCollision2d3(coll_->getVel(), coll_->getMass(), other_->getVel(), other_->getMass(), collisionNormalVec, elasticity);

			
			if (coll_->isDynamic()) {
				if (other_->isDynamic()) {
					float bothRadii = coll_->getRadius() + other_->getRadius();
					float bPart = other_->getRadius() / bothRadii;
					result.posChange = distVec * bPart * Physics::pushoutFactor;
				}
				else {
					result.posChange = distVec * Physics::pushoutFactor;
				}
			}
			//response.velChange = velChange;
		}
	}
	return result;
}

inline std::tuple<bool, float, float> partialSATCollision(Collidable const* coll_, Collidable const* other_)
{
	float resRotation = 0.0f;
	float minClippingDist = 10000000000000.0f;
	for (int i = 0; i < 2; ++i)
	{
		float backRotationDegree = -coll_->getRota() + 90 * i;
		auto backRotatedEntPos = rotate(coll_->getPos(), backRotationDegree);
		float entHalfSize = (i == 0 ? coll_->getSize().x * 0.5f : coll_->getSize().y * 0.5f);
		float minEntProjPos = backRotatedEntPos.x - entHalfSize;
		float maxEntProjPos = backRotatedEntPos.x + entHalfSize;

		std::array<vec2, 4> otherCorners;
		otherCorners[0] = other_->getPos() + rotate(vec2(other_->getSize().x * 0.5f, other_->getSize().y * 0.5f), other_->getRota());
		otherCorners[1] = other_->getPos() + rotate(vec2(other_->getSize().x * 0.5f, -other_->getSize().y * 0.5f), other_->getRota());
		otherCorners[2] = other_->getPos() + rotate(vec2(-other_->getSize().x * 0.5f, other_->getSize().y * 0.5f), other_->getRota());
		otherCorners[3] = other_->getPos() + rotate(vec2(-other_->getSize().x * 0.5f, -other_->getSize().y * 0.5f), other_->getRota());
		for (int j = 0; j < 4; ++j) otherCorners[j] = rotate(otherCorners[j], backRotationDegree);

		auto comp = [](vec2 a, vec2 b) { return a.x < b.x; };
		vec2 minOEntProjPos = *std::min_element(otherCorners.begin(), otherCorners.end(), comp);
		vec2 maxOEntProjPos = *std::max_element(otherCorners.begin(), otherCorners.end(), comp);

		if (doIntervalsOverlap(minEntProjPos, maxEntProjPos, minOEntProjPos.x, maxOEntProjPos.x))
		{
			auto newClippingDist = clippingDist(minEntProjPos, maxEntProjPos, minOEntProjPos.x, maxOEntProjPos.x);
			if (newClippingDist < minClippingDist)
			{
				minClippingDist = newClippingDist;
				resRotation = coll_->getRota() + 270 * i;
				bool orientation = isIntervalCenterSmaller(minEntProjPos, maxEntProjPos, minOEntProjPos.x, maxOEntProjPos.x);
				resRotation += 180 * orientation;
			}
		}
		else
		{
			return std::tuple(false, 0.0f, 0.0f);
		}
	}
	resRotation = float((int)resRotation % 360);
	return std::tuple(true, resRotation, minClippingDist);
}

inline CollisionTestResult rectangleRectangleCollisionCheck(Collidable const* coll_, Collidable const* other_)
{
	CollisionTestResult result = CollisionTestResult();
	auto [partialResult1, rotation1, dist1] = partialSATCollision(coll_, other_);
	if (partialResult1 == true) {
		auto [partialResult2, rotation2, dist2] = partialSATCollision(other_, coll_);

		if (partialResult2)
		{
			result.collided = true;
			auto minClippingDist = dist1 < dist2 ? dist1 : dist2;
			result.clippingDist = minClippingDist;
			if (coll_->isSolid() && other_->isSolid()) {
				
				auto rotation = dist1 < dist2 ? rotation1 : rotation2 + 180;
				rotation = (float)((int)rotation % 360);

				auto collisionNormalVec = vec2(cosf(rotation / RAD), sinf(rotation / RAD));
				result.collisionNormal = collisionNormalVec;
				auto distVec = collisionNormalVec * minClippingDist;

				//float elasticity = std::max(coll_->getElasticity(), other_->getElasticity());
				//auto velChange = dynamicCollision2d3(coll_->getVel(), coll_->getMass(), other_->getVel(), other_->getMass(), collisionNormalVec, elasticity);


				if (coll_->isDynamic()) {
					if (other_->isDynamic()) {
						float BothSizes = coll_->getSize().x * coll_->getSize().y + other_->getSize().x * other_->getSize().y;
						float collPart = coll_->getSize().x * coll_->getSize().y / BothSizes;
						float otherPart = other_->getSize().x * other_->getSize().y / BothSizes;

						result.posChange = distVec * otherPart * Physics::pushoutFactor;
					}
					else {
						result.posChange = distVec * Physics::pushoutFactor;
					}
				}
				//response.velChange = velChange;
			}
		}
	}
	return result;
}

inline CollisionTestResult checkCircleRectangleCollision(Collidable const* circle, Collidable const* rect, bool isCirclePrimary) {
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
	vec2 backRotatedColDir = rotate(collisionDir, rotation);

	if (dist < 0.0f) {
		result.collided = true;
		result.clippingDist = dist;

		if (circle->isSolid() && rect->isSolid()) {
			//float elasticity = std::max(circle->getElasticity(), rect->getElasticity());

			/* the primary is allways dynamic */
			if (isCirclePrimary) {
				//auto velChange = dynamicCollision2d3(circle->getVel(), circle->getMass(), rect->getVel(), rect->getMass(), backRotatedColDir, elasticity);

				result.collisionNormal = backRotatedColDir;
				if (rect->isDynamic()) {
					/* when both are dynamic split the pushback and give each one relativce pushback to their size */
					float bothSizes = circle->getBoundsRadius() * circle->getBoundsRadius() + rect->getBoundsRadius() * rect->getBoundsRadius();
					float circlePart = circle->getBoundsRadius() * circle->getBoundsRadius() / bothSizes;
					float rectPart = rect->getBoundsRadius() * rect->getBoundsRadius() / bothSizes;
					result.posChange = -backRotatedColDir * dist * rectPart * 1.005f;
				}
				else {
					result.posChange = -backRotatedColDir * dist * Physics::pushoutFactor;
				}
				//response.velChange = velChange;
			}
			else {
				//auto velChange = dynamicCollision2d3(rect->getVel(), rect->getMass(), circle->getVel(), circle->getMass(), -backRotatedColDir, elasticity);

				result.collisionNormal = -backRotatedColDir;
				if (circle->isDynamic()) {
					/* when both are dynamic split the pushback and give each one relativce pushback to their size */
					float bothSizes = circle->getBoundsRadius() * circle->getBoundsRadius() + rect->getBoundsRadius() * rect->getBoundsRadius();
					float circlePart = circle->getBoundsRadius() * circle->getBoundsRadius() / bothSizes;
					float rectPart = rect->getBoundsRadius() * rect->getBoundsRadius() / bothSizes;
					result.posChange = backRotatedColDir * dist * circlePart * 1.001f;
				}
				else {
					result.posChange = backRotatedColDir * dist * Physics::pushoutFactor;
				}
				//response.velChange = velChange;
			}
		}
	}
	return result;
}

inline bool isOverlappingAABB(Collidable const* a, Collidable const* b) {
	return fabs(b->getPos().x - a->getPos().x) <= fabs(b->getBoundsSize().x + a->getBoundsSize().x) * 0.5f &&
	fabs(b->getPos().y - a->getPos().y) <= fabs(b->getBoundsSize().y + a->getBoundsSize().y) * 0.5f;
}

inline CollisionTestResult checkForCollision(Collidable const* coll_, Collidable const* other_) {
	//pretest with AABB
	if (isOverlappingAABB(coll_, other_)) {
		if (coll_->getForm() == Form::CIRCLE) {
			if (other_->getForm() == Form::CIRCLE) {
				return circleCircleCollisionCheck(coll_, other_);
			}
			else {
				return checkCircleRectangleCollision(coll_, other_, true);
			}
		}
		else {
			if (other_->getForm() == Form::CIRCLE) {
				return checkCircleRectangleCollision(other_, coll_, false);
			}
			else {
				return rectangleRectangleCollisionCheck(coll_, other_);
			}
		}
	}
	return CollisionTestResult();
}