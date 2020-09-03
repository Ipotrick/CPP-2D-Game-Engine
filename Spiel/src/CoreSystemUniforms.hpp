#pragma once

#include <boost/serialization/access.hpp>

#include "Vec2.hpp"

struct PhysicsUniforms {
	float friction{ 0 };
	Vec2  linearEffectDir{ 0, 0 };
	float linearEffectAccel{ 0 };
	float linearEffectForce{ 0 };
private:
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version)
	{
		ar& friction;
		ar& linearEffectDir;
		ar& linearEffectAccel;
		ar& linearEffectForce;
	}
};