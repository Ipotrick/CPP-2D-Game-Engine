#include "Physics.h"

vec2 calcPosChange(CollidableAdapter const* coll, CollidableAdapter const* other, float const dist, vec2 const& primCollNormal) {
	if (other->isDynamic()) {
		float bothAreas = coll->getSurfaceArea() + other->getSurfaceArea();
		float bPart = other->getSurfaceArea() / bothAreas;
		float collDirV1 = dot(coll->getVel(), primCollNormal);
		float collDirV2 = dot(other->getVel(), primCollNormal);
		if (collDirV1 - collDirV2 > 0.0f && bPart < 0.75f && bPart > 0.25f) {
			//they move into each other
			if (collDirV2 < 0) {
				//coll moved into other
				return dist * primCollNormal * 0.9f + primCollNormal * Physics::nullDelta * 2;
			}
			else {
				//other moved into coll
				return dist * primCollNormal * 0.1f + primCollNormal * Physics::nullDelta * 2;
			}
		}
		else {
			return dist * primCollNormal * bPart + primCollNormal * Physics::nullDelta * 2;
		}
	}
	else {
		return dist * primCollNormal + primCollNormal * Physics::nullDelta * 2;
	}
}

CollisionTestResult checkCircleRectangleCollision(CollidableAdapter const* circle, CollidableAdapter const* rect, bool isCirclePrimary) {
	CollisionTestResult result = CollisionTestResult();

	auto const& rotation = rect->getRota();
	auto rotCirclePos = rotate(circle->getPos(), -rotation);
	auto rotRectPos = rotate(rect->getPos(), -rotation);

	auto rectHalfSize = rect->getSize() * 0.5;

	/* calmp the circle position to the rectangle skin. This is the nbearest point of the circle to the rect. if circle is insided the rectangle, the clamping doesnt work. */
	auto clampedCirclePos = vec2(
		std::max(rotRectPos.x - rectHalfSize.x, std::min(rotCirclePos.x, rotRectPos.x + rectHalfSize.x)),
		std::max(rotRectPos.y - rectHalfSize.y, std::min(rotCirclePos.y, rotRectPos.y + rectHalfSize.y)));

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

	if (dist < 0.0f) {
		vec2 backRotatedCollNormal = rotate(collisionDir, rotation);

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

		result.posChange = calcPosChange(coll, other, -dist, primCollNormal);

	}
	return result;
}

CollisionTestResult rectangleRectangleCollisionCheck(CollidableAdapter const* coll, CollidableAdapter const* other)
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

			auto collNormal = vec2(cosf(rotation / RAD), sinf(rotation / RAD));	// TODO REMOPVE SIN AND COS
			result.collisionNormal = collNormal;

			result.posChange = calcPosChange(coll, other, minClippingDist, collNormal);
		}
	}
	return result;
}

std::tuple<bool, float, float, vec2> partialSATCollision(CollidableAdapter const* coll, CollidableAdapter const* other)
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
			return std::tuple(false, 0.0f, 0.0f, vec2(0, 0));
		}
	}
	resRotation = float((int)resRotation % 360);
	return std::tuple(true, resRotation, minClippingDist, collisionPos);
}

CollisionTestResult circleCircleCollisionCheck(CollidableAdapter const* coll, CollidableAdapter const* other) {
	float dist = circleDist(coll->getPos(), coll->getRadius(), other->getPos(), other->getRadius());

	if (dist < 0.0f) {
		CollisionTestResult result;
		result.collided = true;
		result.clippingDist = dist;
		result.collisionPos = normalize(coll->getPos() - other->getPos()) * other->getRadius() + other->getPos();

		auto collisionNormalVec = normalize(coll->getPos() - other->getPos());	/* a vector from b to coll_ */
		result.collisionNormal = collisionNormalVec;
		result.posChange = calcPosChange(coll, other, -dist, collisionNormalVec);
		return result;
	}
	else {
		return CollisionTestResult();
	}
}

CollisionTestResult checkForCollision(CollidableAdapter const* coll_, CollidableAdapter const* other_) {
	//pretest with AABB
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
			if (isOverlappingAABB(coll_, other_)) {
				return rectangleRectangleCollisionCheck(coll_, other_);
			}
			else {
				return CollisionTestResult();
			}
		}
	}
}