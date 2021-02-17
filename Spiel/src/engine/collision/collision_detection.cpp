#include "collision_detection.hpp"

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

		result.collisionCount = true;
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

	}
	return result;
}


CollisionTestResult rectangleRectangleCollisionCheck2(CollidableAdapter const& coll, CollidableAdapter const& other)
{
	CollisionTestResult result = CollisionTestResult();
	auto result1 = partialSATTest(coll, other);
	if (result1.collisionPointCount) {
		auto result2 = partialSATTest(other, coll);
		if (result2.collisionPointCount) {
			result.collisionCount = true;
			if (result1.minClippingDist < result2.minClippingDist) {
				result.clippingDist = fabs(result1.minClippingDist);
				result.collisionNormal = result1.collisionNormalOfMinClippingDist;
				//result.collisionPos = result1.cornerPosOfMinClippingDist;
				//result.collisionPos2 = result1.cornerPosOfMinClippingDist;
				result.collisionCount = 2;
				result.collisionPos = result1.pos1;
				result.collisionPos2 = result1.pos2;

			}
			else {
				result.clippingDist = fabs(result2.minClippingDist);
				result.collisionNormal = -result2.collisionNormalOfMinClippingDist;	// flip normal vector as cnv allways has to point to coll
				//result.collisionPos = result2.cornerPosOfMinClippingDist;
				//result.collisionPos2 = result2.cornerPosOfMinClippingDist;
				result.collisionCount = 2;
				result.collisionPos = result2.pos2;
				result.collisionPos2 = result2.pos1;
			}
		}
	}
	return result;
}

CollisionTestResult rectangleRectangleCollisionCheck3(CollidableAdapter const& coll, CollidableAdapter const& other)
{
	return CollisionTestResult();
}

int fastMod(int i)
{
	switch (i) {
	case -1: return 3;
	case 0: return 0;
	case 1: return 1;
	case 2: return 2;
	case 3: return 3;
	case 4: return 0;
	default: { assert(false);  return 0; }
	}
}

Vec2 lineLineIntersection(Vec2 o1, Vec2 d1, Vec2 o2, Vec2 d2)
{
	float a1 = d1.y / std::max(0.000001f, d1.x);
	float b1 = o1.y - o1.x * a1;
	float a2 = d2.y / std::max(0.000001f, d2.x);
	float b2 = o2.y - o2.x * a2;
	float x = (b2 - b1) / std::max(0.00001f,(a1 - a2));
	x = clamp(x, o1.x, o1.x + d1.x);
	x = clamp(x, o2.x, o2.x + d2.x);
	float y = a1 * x + b1;
	y = clamp(y, o1.y, o1.y + d1.y);
	y = clamp(y, o2.y, o2.y + d2.y);
	return Vec2(x, y);
}

SATTestResult partialSATTest(CollidableAdapter const& coll, CollidableAdapter const& other) {
	SATTestResult testResult;

	std::array<Vec2, 4> cornersOther{
		other.position + rotate(Vec2(other.size.x * 0.5f,  other.size.y * 0.5f), other.rotationVec), // ur
		other.position + rotate(Vec2(other.size.x * 0.5f, -other.size.y * 0.5f), other.rotationVec), // dr
		other.position + rotate(Vec2(-other.size.x * 0.5f, -other.size.y * 0.5f), other.rotationVec),// dl
		other.position + rotate(Vec2(-other.size.x * 0.5f,  other.size.y * 0.5f), other.rotationVec) // ul
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
			dot(relativePlaneColl, cornersOther[0]), // ur
			dot(relativePlaneColl, cornersOther[1]), // dr
			dot(relativePlaneColl, cornersOther[2]), // dl
			dot(relativePlaneColl, cornersOther[3])  // ul
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
				int index;
				float factor;
				if (isIntervalCenterSmaller(minColl, maxColl, projectedCornersOther[minOtherIndex], projectedCornersOther[maxOtherIndex])) {
					index = minOtherIndex;	// coll is left from other, so the min x pos corner of other is the collision corner
					factor = -1.0f;			// coll is left from other, so the normal vector goes to the left
				}
				else {
					index = maxOtherIndex;	// coll is right from other, so the max x pos corner of other is the collision corner
					factor = 1.0f;			// coll is right from other, so the normal vector goes to the right
				}
				testResult.cornerPosOfMinClippingDist = cornersOther[index];
				testResult.collisionNormalOfMinClippingDist = factor * relativePlaneColl;
				// vectors that point from collisionpoint up to the corners connected to the collision point:
				Vec2 dToLeft  = cornersOther[fastMod(index - 1)] - cornersOther[index];
				Vec2 dToRight = cornersOther[fastMod(index + 1)] - cornersOther[index];

				float stepSizeLeft  = clamp(-testResult.minClippingDist / dot(dToLeft,  testResult.collisionNormalOfMinClippingDist), 0.0f, 1.0f);
				float stepSizeRight = clamp(-testResult.minClippingDist / dot(dToRight, testResult.collisionNormalOfMinClippingDist), 0.0f, 1.0f);
				Vec2 p1 = cornersOther[index] + dToLeft * stepSizeLeft;
				Vec2 p2 = cornersOther[index] + dToRight * stepSizeRight;
				// get corners of normal plane:
				Vec2 normalPlaneLeftCorner  = coll.position - factor * rotate(((i == 0) ? Vec2(coll.size.x * 0.5f, coll.size.y * 0.5f) : Vec2(coll.size.x * 0.5f, coll.size.y * 0.5f)), coll.rotationVec);
				Vec2 normalPlaneRightCorner = coll.position - factor * rotate(((i == 0) ? Vec2(coll.size.x * 0.5f, -coll.size.y * 0.5f) : Vec2(-coll.size.x * 0.5f, coll.size.y * 0.5f)), coll.rotationVec);
				// clamp found points to the normal planes aabb (basicly the same as clipping it to the normal plane):
				Vec2 minAABB = min(normalPlaneLeftCorner, normalPlaneRightCorner);
				Vec2 maxAABB = max(normalPlaneLeftCorner, normalPlaneRightCorner);
				p1.x = clamp(p1.x, minAABB.x, maxAABB.x);
				p1.y = clamp(p1.y, minAABB.y, maxAABB.y);
				p2.x = clamp(p2.x, minAABB.x, maxAABB.x);
				p2.y = clamp(p2.y, minAABB.y, maxAABB.y);
				testResult.pos1 = p1;
				testResult.pos2 = p2;
			}
		}
		else {
			testResult.collisionPointCount = 0;
			return testResult;
		}
	}
	return testResult;
}

CollisionTestResult circleCircleCollisionCheck(CollidableAdapter const& coll, CollidableAdapter const& other) {
	float dist = circleDist(coll.position, coll.size.x / 2.0f, other.position, other.size.x / 2.0f);

	if (dist < 0.0f) {
		CollisionTestResult result;
		result.collisionCount = true;
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
		if (coll.form == Form::Circle) {
			if (other.form == Form::Circle) {
				return circleCircleCollisionCheck(coll, other);
			}
			else {
				return checkCircleRectangleCollision(coll, other, true);
			}
		}
		else {
			if (other.form == Form::Circle) {
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

CollisionTestResult collisionTestCachedAABB(CollidableAdapter const& coll_, CollidableAdapter const& other_) 
{
	if (coll_.form == Form::Circle) {
		if (other_.form == Form::Circle) {
			return circleCircleCollisionCheck(coll_, other_);
		}
		else {
			return checkCircleRectangleCollision(coll_, other_, true);
		}
	}
	else {
		if (other_.form == Form::Circle) {
			return checkCircleRectangleCollision(other_, coll_, false);
		}
		else {
			return rectangleRectangleCollisionCheck2(coll_, other_);
		}
	}
}