#include "collision_detection.h"

CollisionTestResult checkCircleRectangleCollision(CollidableAdapter const& circle, CollidableAdapter const& rect, bool isCirclePrimary) {
	CollisionTestResult result = CollisionTestResult();

	auto rotCirclePos = rotateInverse(circle.position, rect.rotationVec);
	auto rotRectPos = rotateInverse(rect.position, rect.rotationVec);

	auto rectHalfSize = rect.size * 0.5;

	/* calmp the circle position to the rectangle skin. This is the nbearest point of the circle to the rect. if circle is insided the rectangle, the clamping doesnt work. */
	auto clampedCirclePos = Vec2(
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

	Vec2 clampedToCircleCenter = rotCirclePos - clampedCirclePos;
	float clampedToCircleCenterLen = norm(clampedToCircleCenter);
	Vec2 collisionDir;	//allways from rect TO circle
	float dist;
	if (isCircleInsideRect) {
		collisionDir = -normalize(clampedToCircleCenter);
		dist = -clampedToCircleCenterLen - circle.size.x / 2.0f;
	}
	else {
		collisionDir = normalize(clampedToCircleCenter);
		dist = clampedToCircleCenterLen - circle.size.x / 2.0f;
	}

	if (dist < 0.0f) {
		Vec2 backRotatedCollNormal = rotate(collisionDir, rect.rotationVec);

		result.collided = true;
		result.clippingDist = -dist;
		result.collisionPos = rotate(clampedCirclePos, rect.rotationVec);
		CollidableAdapter const* coll; CollidableAdapter const* other; Vec2 primCollNormal;
		if (isCirclePrimary) {
			coll = &circle; other = &rect; primCollNormal = backRotatedCollNormal;
		}
		else {
			coll = &rect; other = &circle; primCollNormal = -backRotatedCollNormal;
		}
		result.collisionNormal = primCollNormal;

		//result.posChange = calcPosChange(*coll, *other, -dist, primCollNormal);

	}
	return result;
}


CollisionTestResult rectangleRectangleCollisionCheck2(CollidableAdapter const& coll, CollidableAdapter const& other)
{
	CollisionTestResult result = CollisionTestResult();
	auto result1 = partialSATTest(coll, other);
	if (!result1.didNotCollide) {
		auto result2 = partialSATTest(other, coll);
		if (!result2.didNotCollide) {
			result.collided = true;
			if (result1.minClippingDist < result2.minClippingDist) {
				result.clippingDist = fabs(result1.minClippingDist);
				result.collisionPos = result1.cornerPosOfMinClippingDist;
				result.collisionNormal = result1.collisionNormalOfMinClippingDist;
			}
			else {
				result.clippingDist = fabs(result2.minClippingDist);
				result.collisionPos = result2.cornerPosOfMinClippingDist;
				result.collisionNormal = -result2.collisionNormalOfMinClippingDist;	// flip normal vector as cnv allways has to point to coll
			}
		}
	}
	return result;
}



SATTestResult partialSATTest(CollidableAdapter const& coll, CollidableAdapter const& other) {
	SATTestResult testResult;

	

	std::array<Vec2, 4> cornersOther{
		other.position + rotate(Vec2(-other.size.x * 0.5f, -other.size.y * 0.5f), other.rotationVec), // ul
		other.position + rotate(Vec2( other.size.x * 0.5f, -other.size.y * 0.5f), other.rotationVec), // ur
		other.position + rotate(Vec2(-other.size.x * 0.5f,  other.size.y * 0.5f), other.rotationVec), // dl
		other.position + rotate(Vec2( other.size.x * 0.5f,  other.size.y * 0.5f), other.rotationVec)  // dr
	};

	for (int i = 0; i < 2; i++) {
		Vec2 relativePlaneColl;
		float minColl;
		float maxColl;
		if (i == 0) { // phase 1: project to relative x plane of coll0
			relativePlaneColl = rotate(Vec2(1, 0), coll.rotationVec);
			minColl = dot(relativePlaneColl, coll.position) - coll.size.x * 0.5f;
			maxColl = dot(relativePlaneColl, coll.position) + coll.size.x * 0.5f;
		}
		else { // phase 2: project to relative y plane of coll
			relativePlaneColl = rotate(Vec2(0, 1), coll.rotationVec);
			minColl = dot(relativePlaneColl, coll.position) - coll.size.y * 0.5f;
			maxColl = dot(relativePlaneColl, coll.position) + coll.size.y * 0.5f;
		}

		std::array<float, 4> projectedCornersOther{
			dot(relativePlaneColl, cornersOther[0]), // ul
			dot(relativePlaneColl, cornersOther[1]), // ur
			dot(relativePlaneColl, cornersOther[2]), // dl
			dot(relativePlaneColl, cornersOther[3])  // dr
		};

		int minOtherIndex = 0;
		int maxOtherIndex = 0;
		for (int i = 1; i < 4; i++) {
			if (projectedCornersOther[i] < projectedCornersOther[minOtherIndex]) minOtherIndex = i;
			if (projectedCornersOther[i] > projectedCornersOther[maxOtherIndex]) maxOtherIndex = i;
		}

		if (doIntervalsOverlap(minColl, maxColl, projectedCornersOther[minOtherIndex], projectedCornersOther[maxOtherIndex])) {
			auto newClippingDist = clippingDist(minColl, maxColl, projectedCornersOther[minOtherIndex], projectedCornersOther[maxOtherIndex]);
			if (newClippingDist < testResult.minClippingDist) {
				testResult.minClippingDist = newClippingDist;
				// find corner of other, that is nearest to coll's center
				if (isIntervalCenterSmaller(minColl, maxColl, projectedCornersOther[minOtherIndex], projectedCornersOther[maxOtherIndex])) {
					testResult.cornerPosOfMinClippingDist = cornersOther[minOtherIndex]; // coll is left from other, so the min x pos corner of other is the collision corner
					testResult.collisionNormalOfMinClippingDist = -relativePlaneColl; // coll is left from other, so the normal vector goes to the left
				}
				else {
					testResult.cornerPosOfMinClippingDist = cornersOther[maxOtherIndex]; // coll is right from other, so the max x pos corner of other is the collision corner
					testResult.collisionNormalOfMinClippingDist = relativePlaneColl; // coll is right from other, so the normal vector goes to the right
				}
			}
		}
		else {
			testResult.didNotCollide = true;
			return testResult;
		}
	}
	return testResult;
}

CollisionTestResult circleCircleCollisionCheck(CollidableAdapter const& coll, CollidableAdapter const& other) {
	float dist = circleDist(coll.position, coll.size.x / 2.0, other.position, other.size.x / 2.0);

	if (dist < 0.0f) {
		CollisionTestResult result;
		result.collided = true;
		result.clippingDist = -dist;
		result.collisionPos = normalize(coll.position - other.position) * other.size.x / 2.0f + other.position;

		auto collisionNormalVec = normalize(coll.position - other.position);	/* a vector from b to coll_ */
		result.collisionNormal = collisionNormalVec;
		//result.posChange = calcPosChange(coll, other, -dist, collisionNormalVec);
		return result;
	}
	else {
		return CollisionTestResult();
	}
}

CollisionTestResult collisionTest(CollidableAdapter const& coll, CollidableAdapter const& other) {
	//pretest with AABB

	if (isOverlappingAABB(coll.position, aabbBounds(coll.size, coll.rotationVec), other.position, aabbBounds(other.size, other.rotationVec))) {
		if (coll.form == Form::CIRCLE) {
			if (other.form == Form::CIRCLE) {
				return circleCircleCollisionCheck(coll, other);
			}
			else {
				return checkCircleRectangleCollision(coll, other, true);
			}
		}
		else {
			if (other.form == Form::CIRCLE) {
				return checkCircleRectangleCollision(other, coll, false);
			}
			else {
				return rectangleRectangleCollisionCheck2(coll, other);
			}
		}
	}
	else {
		return CollisionTestResult();
	}
}

CollisionTestResult collisionTestCachedAABB(CollidableAdapter const& coll_, CollidableAdapter const& other_, Vec2 const aabbColl, Vec2 const aabbOther) {
	//pretest with AABB

	if (isOverlappingAABB(coll_.position, aabbColl, other_.position, aabbOther)) {
		if (coll_.form == Form::CIRCLE) {
			if (other_.form == Form::CIRCLE) {
				return circleCircleCollisionCheck(coll_, other_);
			}
			else {
				return checkCircleRectangleCollision(coll_, other_, true);
			}
		}
		else {
			if (other_.form == Form::CIRCLE) {
				return checkCircleRectangleCollision(other_, coll_, false);
			}
			else {
				return rectangleRectangleCollisionCheck2(coll_, other_);
			}
		}
	}
	else {
		return CollisionTestResult();
	}
}