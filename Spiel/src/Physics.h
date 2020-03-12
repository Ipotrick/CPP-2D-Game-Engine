#pragma once

#include <vector>
#include <array>

#include "glmath.h"
#include "Entity.h"

namespace Physics {
	constexpr float maxMass = 1'000'000'000'000.f;
	constexpr float nullDelta = 0.00001f;

	inline std::vector<Drawable> debugDrawables;
}

struct CollisionResponse {
	vec2 posChange;
	vec2 velChange;
	bool collided;
	float clippingDist;

	CollisionResponse() : posChange{ 0.0f }, velChange{ 0.0f }, collided{ false }, clippingDist{ 0.0f } {}
};

struct CollisionInfo {
	uint32_t idA;
	uint32_t idB;
	float clippingDist;

	CollisionInfo(uint32_t idA_, uint32_t idB_, float clippingDist_) :idA{ idA_ }, idB{ idB_ }, clippingDist{ clippingDist_ } {}
};

inline CollisionResponse operator+(CollisionResponse a, CollisionResponse b) {
	a.collided |= b.collided;
	a.posChange += b.posChange;
	a.velChange += b.velChange;
	return a;
}

inline std::tuple<float, float> dynamicCollision2(float m1, float u2, float m2, float e)
{
	float firstFactor = m2 * u2;
	float secondFactor = m1 + m2;
	return std::tuple<float, float>(
		(firstFactor + m2 * e * u2) / (secondFactor),
		(firstFactor + m1 * e * -u2) / (secondFactor)
		);
}

/* retruns the change in velocity for the first entered entity in an elastic collision*/
inline vec2 dynamicCollision2d2(vec2 const& u1, float const& m1, vec2 const& u2, float const& m2, vec2 const& cNV, float elasticity) {
	/* bezugssystem ist u2 */
	auto a_u2 = u2 - u1;
	float pu2 = dot(a_u2, cNV);

	bool areTheyGoingIntoEachOther = false;
	if (pu2 > -Physics::nullDelta) {
		areTheyGoingIntoEachOther = true;
	}
	if (areTheyGoingIntoEachOther) {
		auto [cu1, cu2] = dynamicCollision2(m1, pu2, m2, elasticity);
		return ((cu1)* cNV);
	}
	else {	//die collidables bewegen sich nicht aufeinander zu es ist nur clipping und keine collision
		return 0;
	}
}

inline float circleDist(vec2 const pos1, float rad1, vec2 const pos2, float rad2)
{
	float distCaer = distance(pos1, pos2);
	return distCaer - (rad1 + rad2);
}

inline bool isIntervalCenterSmaller(float minFirst, float  maxFirst, float  minSecond, float  maxSecond)
{
	auto centerFirst = (maxFirst - minFirst) * 0.5f + minFirst;
	auto centerSecond = (maxSecond - minSecond) * 0.5f + minSecond;
	return (centerFirst < centerSecond);
}

inline bool doIntervalsOverlap(float minFirst, float  maxFirst, float  minSecond, float  maxSecond)
{
	return !(minFirst > maxSecond || minSecond > maxFirst);
}

inline float clippingDist(float minFirst, float  maxFirst, float  minSecond, float  maxSecond)
{
	if (minFirst > maxSecond || minSecond > maxFirst)
	{
		return 0.0f;
	}
	else
	{
		std::array<float, 2> distances{ fabs(minFirst - maxSecond), fabs(maxFirst - minSecond) };
		return *std::min_element(distances.begin(), distances.end());
	}
}

inline CollisionResponse circleCircleCollisionCheck(Collidable const* coll_, Collidable const* other_)  {
	CollisionResponse response = CollisionResponse();
	float dist = circleDist(coll_->getPos(), coll_->getRadius(), other_->getPos(), other_->getRadius());

	if (dist < 0.0f)
	{
		response.collided = true;
		response.clippingDist = dist;
		if (coll_->isSolid() && other_->isSolid()) {
			auto collisionNormalVec = normalize(coll_->getPos() - other_->getPos());	/* a vector from b to coll_ */ 
			auto circleOverlap = -dist;
			auto distVec = collisionNormalVec * circleOverlap;

			float elasticity = std::max(coll_->getElasticity(), other_->getElasticity());
			auto velChange = dynamicCollision2d2(coll_->getVel(), coll_->getMass(), other_->getVel(), other_->getMass(), collisionNormalVec, elasticity);

			
			if (coll_->isDynamic()) {
				if (other_->isDynamic()) {
					float bothRadii = coll_->getRadius() + other_->getRadius();
					float bPart = other_->getRadius() / bothRadii;
					response.posChange = distVec * bPart * 1.001f;
				}
				else {
					response.posChange = distVec * 1.001f;
				}
			}
			response.velChange = velChange;
		}
	}
	return response;
}

inline std::tuple<bool, float, float> partialSATCollision(Collidable const* coll_, Collidable const* other_)
{
	float resRotation = 0.0f;
	float minClippingDist = 10000000000000.0f;
	for (int i = 0; i < 2; ++i)
	{
		float backRotationDegree = -coll_->getRota() + 90 * i;
		auto backRotatedEntPos = rotate(coll_->getPos(), backRotationDegree);
		float entHalfSize = (i == 0 ? coll_->getHitboxSize().x * 0.5f : coll_->getHitboxSize().y * 0.5f);
		float minEntProjPos = backRotatedEntPos.x - entHalfSize;
		float maxEntProjPos = backRotatedEntPos.x + entHalfSize;

		std::array<vec2, 4> otherCorners;
		otherCorners[0] = other_->getPos() + rotate(vec2(other_->getHitboxSize().x * 0.5f, other_->getHitboxSize().y * 0.5f), other_->getRota());
		otherCorners[1] = other_->getPos() + rotate(vec2(other_->getHitboxSize().x * 0.5f, -other_->getHitboxSize().y * 0.5f), other_->getRota());
		otherCorners[2] = other_->getPos() + rotate(vec2(-other_->getHitboxSize().x * 0.5f, other_->getHitboxSize().y * 0.5f), other_->getRota());
		otherCorners[3] = other_->getPos() + rotate(vec2(-other_->getHitboxSize().x * 0.5f, -other_->getHitboxSize().y * 0.5f), other_->getRota());
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

inline CollisionResponse rectangleRectangleCollisionCheck(Collidable const* coll_, Collidable const* other_)
{
	CollisionResponse response = CollisionResponse();
	auto [partialResult1, rotation1, dist1] = partialSATCollision(coll_, other_);
	if (partialResult1 == true) {
		auto [partialResult2, rotation2, dist2] = partialSATCollision(other_, coll_);

		if (partialResult2)
		{
			response.collided = true;
			auto minClippingDist = dist1 < dist2 ? dist1 : dist2;
			response.clippingDist = minClippingDist;
			if (coll_->isSolid() && other_->isSolid()) {
				
				auto rotation = dist1 < dist2 ? rotation1 : rotation2 + 180;
				rotation = (float)((int)rotation % 360);

				auto collisionNormalVec = vec2(cosf(rotation / RAD), sinf(rotation / RAD));
				auto distVec = collisionNormalVec * minClippingDist;

				float elasticity = std::max(coll_->getElasticity(), other_->getElasticity());
				auto velChange = dynamicCollision2d2(coll_->getVel(), coll_->getMass(), other_->getVel(), other_->getMass(), collisionNormalVec, elasticity);


				if (coll_->isDynamic()) {
					if (other_->isDynamic()) {
						float BothSizes = coll_->getHitboxSize().x * coll_->getHitboxSize().y + other_->getHitboxSize().x * other_->getHitboxSize().y;
						float collPart = coll_->getHitboxSize().x * coll_->getHitboxSize().y / BothSizes;
						float otherPart = other_->getHitboxSize().x * other_->getHitboxSize().y / BothSizes;

						response.posChange = distVec * otherPart * 1.001f;
					}
					else {
						response.posChange = distVec * 1.001f;
					}
				}
				response.velChange = velChange;
			}
		}
	}
	return response;
}

inline CollisionResponse checkCircleRectangleCollision(Collidable const* circle, Collidable const* rect, bool isCirclePrimary) {
	CollisionResponse response = CollisionResponse();

	auto const& rotation = rect->getRota();
	auto rotCirclePos = rotate(circle->getPos(), -rotation);
	auto rotRectPos = rotate(rect->getPos(), -rotation);

	auto rectHalfSize = rect->getHitboxSize() * 0.5;

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
		response.collided = true;
		response.clippingDist = dist;

		if (circle->isSolid() && rect->isSolid()) {
			float elasticity = std::max(circle->getElasticity(), rect->getElasticity());

			/* the primary is allways dynamic */
			if (isCirclePrimary) {
				auto velChange = dynamicCollision2d2(circle->getVel(), circle->getMass(), rect->getVel(), rect->getMass(), backRotatedColDir, elasticity);
				if (rect->isDynamic()) {
					/* when both are dynamic split the pushback and give each one relativce pushback to their size */
					float bothSizes = circle->getBoundsRadius() * circle->getBoundsRadius() + rect->getBoundsRadius() * rect->getBoundsRadius();
					float circlePart = circle->getBoundsRadius() * circle->getBoundsRadius() / bothSizes;
					float rectPart = rect->getBoundsRadius() * rect->getBoundsRadius() / bothSizes;
					response.posChange = -backRotatedColDir * dist * rectPart * 1.001f;
				}
				else {
					response.posChange = -backRotatedColDir * dist * 1.001f;
				}
				response.velChange = velChange;
			}
			else {
				auto velChange = dynamicCollision2d2(rect->getVel(), rect->getMass(), circle->getVel(), circle->getMass(), -backRotatedColDir, elasticity);
				if (circle->isDynamic()) {
					/* when both are dynamic split the pushback and give each one relativce pushback to their size */
					float bothSizes = circle->getBoundsRadius() * circle->getBoundsRadius() + rect->getBoundsRadius() * rect->getBoundsRadius();
					float circlePart = circle->getBoundsRadius() * circle->getBoundsRadius() / bothSizes;
					float rectPart = rect->getBoundsRadius() * rect->getBoundsRadius() / bothSizes;
					response.posChange = backRotatedColDir * dist * circlePart * 1.001f;
				}
				else {
					response.posChange = backRotatedColDir * dist * 1.001f;
				}
				response.velChange = velChange;
			}
		}
	}
	return response;
}

inline bool isOverlappingAABB(Collidable const* a, Collidable const* b) {
	return fabs(b->getPos().x - a->getPos().x) <= fabs(b->getBoundsSize().x + a->getBoundsSize().x) * 0.5f &&
	fabs(b->getPos().y - a->getPos().y) <= fabs(b->getBoundsSize().y + a->getBoundsSize().y) * 0.5f;
}

inline CollisionResponse checkForCollision(Collidable const* coll_, Collidable const* other_) {
	if (coll_ == other_) return CollisionResponse();
	//pretest with AABB
	if (isOverlappingAABB(coll_, other_)) {
		if (coll_->getForm() == Collidable::Form::CIRCLE) {
			if (other_->getForm() == Collidable::Form::CIRCLE) {
				return circleCircleCollisionCheck(coll_, other_);
			}
			else {
				return checkCircleRectangleCollision(coll_, other_, true);
			}
		}
		else {
			if (other_->getForm() == Collidable::Form::CIRCLE) {
				return checkCircleRectangleCollision(other_, coll_, false);
			}
			else {
				return rectangleRectangleCollisionCheck(coll_, other_);
			}
		}
	}
	return CollisionResponse();
}