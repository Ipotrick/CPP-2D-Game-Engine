#pragma once

#include <vector>
#include <array>

#include "glmath.h"
#include "Collidable.h"

namespace Physics {
	constexpr float maxMass = 1'000'000'000'000.f;
	constexpr float nullDelta = 0.00001f;
}

struct CollisionResponse {
	vec2 posChange;
	vec2 velChange;
	bool collided;

	CollisionResponse() : posChange{ 0.0f }, velChange{ 0.0f }, collided{ false } {}
};

struct CollisionInfo {
	uint32_t idA;
	uint32_t idB;

	CollisionInfo(uint32_t idA_, uint32_t idB_) :idA{idA_}, idB{idB_} {}
};

inline CollisionResponse operator+(CollisionResponse a, CollisionResponse b) {
	a.collided |= b.collided;
	a.posChange += b.posChange;
	a.velChange += b.velChange;
	return a;
}

inline std::tuple<float, float> dynamicCollision(float u1, float m1, float u2, float m2, float e)
{
	float firstFactor = m1 * u1 + m2 * u2;
	float secondFactor = m1 + m2;
	return std::tuple<float, float>(
		(firstFactor + m2 * e * (u2 - u1)) / (secondFactor),
		(firstFactor + m1 * e * (u1 - u2)) / (secondFactor)
		);
}

inline std::tuple<vec2, vec2> dynamicCollision2d2(vec2 const& u1, float const& m1, vec2 const& u2, float const& m2, vec2 const& cNV, float elasticity) {
	vec2 ocNV = rotate(cNV, 90);
	/* bezugssystem ist u2 */
	auto a_u2 = u2 - u1;
	float pu2 = dot(a_u2, cNV);
	vec2 ru2 = (a_u2 * ocNV) * ocNV;

	bool areTheyGoingIntoEachOther = false;
	if (pu2 > -Physics::nullDelta) {
		areTheyGoingIntoEachOther = true;
	}
	if (areTheyGoingIntoEachOther) {
		auto [cu1, cu2] = dynamicCollision(0, m1, pu2, m2, elasticity);
		return std::tuple<vec2, vec2>(
			((cu1) * cNV) + u1,
			((cu2) * cNV) + ru2 + u1);
	}
	else {	//die collidables bewegen sich nicht aufeinander zu es ist nur clipping und keine collision
		return std::tuple<vec2, vec2>(
			u1, u2
			);
	}
}

/*
inline std::tuple<vec2, vec2> dynamicCollision2d(vec2 const& u1, float const& m1, vec2 const& u2, float const& m2, vec2 const& cNV, float elasticity) 
{
	vec2 ocNV = rotate(cNV, 90);	// orthogonal collision normal vector 
	float pu1 = dot(u1, cNV); float pu2 = dot(u2, cNV);
	vec2 ru1 = (u1 * ocNV) * ocNV; vec2 ru2 = (u2 * ocNV) * ocNV;

	bool areTheyGoingIntoEachOther = false;
	float correcture = 0;

	if (pu1 < Physics::nullDelta && pu2 < Physics::nullDelta) {
		if (abs(pu1) > abs(pu2)) areTheyGoingIntoEachOther = true;	//pu1 followes and catches pu2
		correcture = -(pu2 + Physics::nullDelta);
	}
	if (pu1 > -Physics::nullDelta && pu2 > -Physics::nullDelta) {
		if (abs(pu2) > abs(pu1)) areTheyGoingIntoEachOther = true;	//pu2 followes and catches pu1
	}
	if (pu1 < Physics::nullDelta && pu2 > -Physics::nullDelta) areTheyGoingIntoEachOther = true;

	if (areTheyGoingIntoEachOther) {
		auto [cu1, cu2] = dynamicCollision(pu1 + correcture, m1, pu2 + correcture, m2, elasticity);
		return std::tuple<vec2, vec2>(
			((cu1 - correcture) * cNV) + ru1,
			((cu2 - correcture) * cNV) + ru2);
	}
	else {	//die collidables bewegen sich nicht aufeinander zu es ist nur clipping und keine collision
		return std::tuple<vec2, vec2>(
			u1,u2
			);
	}
}*/

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
		if (coll_->isSolid()) {
			auto collisionNormalVec = normalize(coll_->getPos() - other_->getPos());	/* a vector from b to coll_ */ 
			auto circleOverlap = -dist;
			auto distVec = collisionNormalVec * circleOverlap;

			float elasticity = std::max(coll_->getElasticity(), other_->getElasticity());
			auto [v1, v2] = dynamicCollision2d2(coll_->getVel(), coll_->getMass(), other_->getVel(), other_->getMass(), collisionNormalVec, elasticity);

			
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
			//vec change is newVel - oldVel	= v1 - coll_->getVel()
			response.velChange = (v1 - coll_->getVel());
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
	auto [partialResult1, rotation1, dist1] = partialSATCollision(coll_, other_);
	auto [partialResult2, rotation2, dist2] = partialSATCollision(other_, coll_);

	CollisionResponse response = CollisionResponse();

	if (partialResult1 && partialResult2)
	{
		response.collided = true;
		if (coll_->isSolid()) {
			auto minClippingDist = dist1 < dist2 ? dist1 : dist2;
			auto rotation = dist1 < dist2 ? rotation1 : rotation2 + 180;
			rotation = (float)((int)rotation % 360);

			auto collisionNormalVec = vec2(cos(rotation / RAD), sin(rotation / RAD));
			auto distVec = collisionNormalVec * minClippingDist;

			float elasticity = std::max(coll_->getElasticity(), other_->getElasticity());
			auto [v1, v2] = dynamicCollision2d2(coll_->getVel(), coll_->getMass(), other_->getVel(), other_->getMass(), collisionNormalVec, elasticity);


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
			//vec change is newVel - oldVel	= v1 - coll_->getVel()
			response.velChange = (v1 - coll_->getVel());
		}
	}
	return response;
}

inline CollisionResponse doCircleRectangleCollision(Collidable const* coll_, Collidable const* other_, bool isCollPrimary_)
{
	CollisionResponse response = CollisionResponse();
	/* check if they really collided */
	/* rotate circle and rec so that the rect is axis aligned */
	auto circlePos = coll_->getPos();
	auto circleSpeed = coll_->getVel();
	auto circleRad = coll_->getRadius();
	auto rectPos = other_->getPos();
	auto rectHalfSize = other_->getHitboxSize() * 0.5f;
	auto rectSpeed = other_->getVel();
	auto rotation = other_->getRota();
	/* correct rotation */
	circlePos = rotate(circlePos, -rotation);
	circleSpeed = rotate(circleSpeed, -rotation);
	rectPos = rotate(rectPos, -rotation);
	rectSpeed = rotate(rectSpeed, -rotation);
	/* clamp the circle position to the rectangle */
	auto circleProjToRectX = std::max(rectPos.x - rectHalfSize.x, std::min(circlePos.x, rectPos.x + rectHalfSize.x));
	auto circleProjToRectY = std::max(rectPos.y - rectHalfSize.y, std::min(circlePos.y, rectPos.y + rectHalfSize.y));
	auto clampedCirclePos = vec2(circleProjToRectX, circleProjToRectY);
	/* if the circle is inside the rectangle, the nearest pos of the circle to a wall needs to be calculated differenly */
	bool isCircleInsideRect{ false };
	if (clampedCirclePos.x > rectPos.x - rectHalfSize.x && clampedCirclePos.x < rectPos.x + rectHalfSize.x
		&& clampedCirclePos.y > rectPos.y - rectHalfSize.y && clampedCirclePos.y < rectPos.y + rectHalfSize.y)
	{
		auto differenceXLeft = fabs(circlePos.x - (rectPos.x - rectHalfSize.x));
		auto differenceXRight = fabs(circlePos.x - (rectPos.x + rectHalfSize.x));
		auto differenceYLeft = fabs(circlePos.y - (rectPos.y - rectHalfSize.y));
		auto differenceYRight = fabs(circlePos.y - (rectPos.y + rectHalfSize.y));
		if (differenceXLeft < differenceXRight && differenceXLeft < differenceYLeft && differenceXLeft < differenceYRight)
		{
			clampedCirclePos.x = rectPos.x - rectHalfSize.x;
		}
		else if (differenceXRight < differenceXLeft && differenceXRight < differenceYLeft && differenceXRight < differenceYRight)
		{
			clampedCirclePos.x = rectPos.x + rectHalfSize.x;
		}
		else if (differenceYLeft < differenceXLeft && differenceYLeft < differenceXRight && differenceYLeft < differenceYRight)
		{
			clampedCirclePos.y = rectPos.y - rectHalfSize.y;
		}
		else
		{
			clampedCirclePos.y = rectPos.y + rectHalfSize.y;
		}
		isCircleInsideRect = true;
	}

	auto distanceVec = clampedCirclePos - circlePos;
	/* seperate the distVec into a distance and a direction */
	auto dist = norm(distanceVec) - circleRad;
	vec2 normDirVec = distanceVec;
	normDirVec = normalize(normDirVec);
	vec2 backRotatedNormDirVec = normDirVec;
	normDirVec = rotate(normDirVec, rotation);


	if (isCircleInsideRect == true)
	{
		response.collided = true;
		if (coll_->isSolid()) {
			float elasticity = std::max(coll_->getElasticity(), other_->getElasticity());
			auto[v1, v2] = dynamicCollision2d2(coll_->getVel(), coll_->getMass(), other_->getVel(), other_->getMass(), backRotatedNormDirVec, elasticity);

			if (coll_->isDynamic()) {
				if (other_->isDynamic()) {
					auto bothSizes = coll_->getBoundsRadius() + other_->getBoundsRadius();
					auto collPart = coll_->getBoundsRadius() / bothSizes;
					auto otherPart = other_->getBoundsRadius() / bothSizes;

					if (isCollPrimary_) {
						response.posChange = (normDirVec * (dist + circleRad * 2)) * otherPart * 1.001f;
					}
					else {
						response.posChange = -(normDirVec * (dist + circleRad * 2)) * collPart * 1.001f;
					}
				}
				else {
					if (isCollPrimary_) {
						response.posChange = (normDirVec * (dist + circleRad * 2)) * 1.001f;
					}
					else {
						response.posChange = -(normDirVec * (dist + circleRad * 2)) * 1.001f;
					}
				}
			}
			if (isCollPrimary_) {
				response.velChange = (v1 - coll_->getVel());
			}
			else {
				response.velChange = (v2 - other_->getVel());
			}
		}
	}
	else if (dist < 0.0f)
	{
		response.collided = true;
		if (coll_->isSolid()) {
			float elasticity = std::max(coll_->getElasticity(), other_->getElasticity());
			auto [v1, v2] = dynamicCollision2d2(coll_->getVel(), coll_->getMass(), other_->getVel(), other_->getMass(), -backRotatedNormDirVec, elasticity);

			if (coll_->isDynamic()) {
				if (other_->isDynamic()) {
					auto bothSizes = coll_->getBoundsRadius() + other_->getBoundsRadius();
					auto collPart = coll_->getBoundsRadius() / bothSizes;
					auto otherPart = other_->getBoundsRadius() / bothSizes;

					if (isCollPrimary_) {
						response.posChange = normDirVec * dist * otherPart * 1.001f;
					}
					else {
						response.posChange = -normDirVec * dist * collPart * 1.001f;
					}
				}
				else {
					if (isCollPrimary_) {
						response.posChange = normDirVec * dist * 1.001f;
					}
					else {
						response.posChange = -normDirVec * dist * 1.001f;
					}
				}
				
			}
			if (isCollPrimary_) {
				response.velChange = (v1 - coll_->getVel());
			}
			else {
				response.velChange = (v2 - other_->getVel());
			}
		}
	}
	return response;
}


inline CollisionResponse checkForCollisions(Collidable const* coll_, std::vector<Collidable*> const& others_) {
	CollisionResponse result = CollisionResponse();
	for (auto const& other : others_) {
		if (coll_ == other) continue;

		if (coll_->getForm() == Collidable::Form::CIRCLE) {
			if (other->getForm() == Collidable::Form::CIRCLE) {
				result = result + circleCircleCollisionCheck(coll_, other);
			}
			else {
				/* circle, rectangle */
				result = result + CollisionResponse();
			}
		}
		else {
			if (other->getForm() == Collidable::Form::CIRCLE) {
				/* rectangle, circle */
				result = result + CollisionResponse();
			}
			else {
				/* rectangle, rectangle */
				result = result + rectangleRectangleCollisionCheck(coll_, other);
			}
		}
	}
	return result;
}

inline CollisionResponse checkForCollision(Collidable const* coll_, Collidable const* other_) {
	if (coll_ == other_) return CollisionResponse();

	if (coll_->getForm() == Collidable::Form::CIRCLE) {
		if (other_->getForm() == Collidable::Form::CIRCLE) {
			return circleCircleCollisionCheck(coll_, other_);
		}
		else {
			return doCircleRectangleCollision(coll_, other_, true);
		}
	}
	else {
		if (other_->getForm() == Collidable::Form::CIRCLE) {

			auto puffer = doCircleRectangleCollision(other_, coll_, false);
			return puffer;
		}
		else {
			return rectangleRectangleCollisionCheck(coll_, other_);
		}
	}
}