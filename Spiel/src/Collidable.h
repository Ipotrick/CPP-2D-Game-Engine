#pragma once

#include <algorithm> 
#include "glmath.h"
#include "Basis.h"

class Collidable : virtual public Basis {
public:
	enum class Form {
		CIRCLE,
		RECTANGLE
	};
protected:
	inline static int nextId = 0;
	int id;
	bool dynamic;
public:
	Collidable(vec2 size_ = vec2(1, 1), Form form_ = Form::CIRCLE, float elasticity_ = 1, bool dynamic_ = false, float mass_ = 1, vec2 velocity_ = vec2(0, 0)) :
		Basis{},
		id{ nextId++ },
		hitboxSize{ size_ },
		hitboxForm{ form_ },
		elasticity{ elasticity_ },
		dynamic{ dynamic_ },
		mass{ mass_ },
		velocity{ velocity_ },
		collided{ false },
		solid{ true }
	{
	}

	inline vec2 getVel() const { return velocity; }
	inline Form getForm() const { return hitboxForm; }
	inline vec2 getHitboxSize() const { return hitboxSize; }
	inline float getRadius() const { assert(hitboxForm == Form::CIRCLE); return hitboxSize.r / 2; }
	inline float getMass() const { return mass; }
	inline float getElasticity() const { return elasticity; }

	inline uint32_t getId() const { return id; }
	inline bool isCollided() const { return collided; }
	inline bool isDynamic() const { return dynamic; }
	inline bool isSolid() const { return solid; }

	inline Collidable* getCollidablePtr() { return this; }
	inline Collidable const* getConstCollidablePtr() const { return this; }
	vec2 getBoundsSize();
	float getBoundsRadius();

public:
	Form hitboxForm;
	vec2 hitboxSize;
	vec2 velocity;
	float mass;
	float elasticity;
	bool collided;
	bool solid;
};





inline vec2 Collidable::getBoundsSize() {
	if (hitboxForm == Form::CIRCLE)
	{
		return vec2(getBoundsRadius() * 2);
	}
	else
	{
		return vec2(getBoundsRadius() * 2);
	}
}

inline float Collidable::getBoundsRadius() {
	if (hitboxForm == Form::CIRCLE) {
		return hitboxSize.r/2.0f;
	}
	else {

		return sqrtf((hitboxSize.x * hitboxSize.x + hitboxSize.y * hitboxSize.y)) / 2.0f;
	}
}