#pragma once

#include <algorithm> 
#include <atomic>

#include "glmath.h"

enum class Form {
	CIRCLE = 0x00,
	RECTANGLE = 0x01
};

struct Basis {
	/* x y world coordinates, z depth*/
	vec2 position;
	/* in radiants 2pi = one rotation*/
	float rotation;

	Basis() :
		position{ 0.0f, 0.0f },
		rotation{ 0.0f }
	{
	}

	Basis(vec2 position_, float rotation_) :
		position{ position_ },
		rotation{ rotation_ }
	{
	}

	inline vec2 getPos() const { return position; }
	inline float getRota() const { return rotation; }
};



class Collidable : virtual public Basis {
public:
protected:
	bool dynamic;
public:
	Collidable(vec2 size_, Form form_, bool solid_, bool dynamic_, float elasticity_ = 1,  float mass_ = 1, vec2 velocity_ = vec2(0, 0)) :
		Basis{},
		hitboxSize{ size_ },
		hitboxForm{ form_ },
		elasticity{ elasticity_ },
		dynamic{ dynamic_ },
		mass{ mass_ },
		velocity{ velocity_ },
		acceleration{ 0,0 },
		collided{ false },
		solid{ solid_ }
	{
	}

	Collidable(Collidable const& c) :
		Basis{c.getPos(), c.getRota()},
		hitboxSize{ c.getHitboxSize() },
		hitboxForm{ c.getForm() },
		elasticity{ c.elasticity },
		dynamic{ c.isDynamic() },
		mass{ c.getMass() },
		velocity{ c.getVel() },
		acceleration{ 0,0 },
		collided{ false },
		solid{ c.isSolid() }
	{
	}

	inline vec2 getVel() const { return velocity; }
	inline vec2 getAcl() const { return acceleration; }
	inline Form getForm() const { return hitboxForm; }
	inline vec2 getHitboxSize() const { return hitboxSize; }
	inline float getRadius() const { assert(hitboxForm == Form::CIRCLE); return hitboxSize.r / 2; }
	inline float getMass() const { return mass; }
	inline float getElasticity() const { return elasticity; }

	inline bool isCollided() const { return collided; }
	inline bool isDynamic() const { return dynamic; }
	inline bool isSolid() const { return solid; }

	inline Collidable* getCollidablePtr() { return this; }
	inline Collidable const* getConstCollidablePtr() const { return this; }
	vec2 getBoundsSize() const;
	float getBoundsRadius() const;

public:
	Form hitboxForm;
	vec2 hitboxSize;
	vec2 velocity;
	vec2 acceleration;
	float mass;
	float elasticity;
	bool collided;
	bool solid;
};

inline vec2 Collidable::getBoundsSize() const {
	if (hitboxForm == Form::CIRCLE)
	{
		return vec2(getBoundsRadius() * 2);
	}
	else
	{
		return vec2(getBoundsRadius() * 2);
	}
}

inline float Collidable::getBoundsRadius() const {
	if (hitboxForm == Form::CIRCLE) {
		return hitboxSize.r / 2.0f;
	}
	else {

		return sqrtf((hitboxSize.x * hitboxSize.x + hitboxSize.y * hitboxSize.y)) / 2.0f;
	}
}



class Drawable : virtual public Basis {
public:
	vec4 color;
	vec2 scale;
	float drawingPrio;
	uint32_t id;
	Form form;
	bool throwsShadow;

	Drawable(uint32_t id_, vec2 position_, float drawingPrio_, vec2 scale_, vec4 color_, Form form_, float rotation_, bool throwsShadow_ = false) :
		Basis(vec2(position_.x, position_.y), rotation_),
		drawingPrio{ drawingPrio_ },
		scale{ scale_ },
		color{ color_ },
		id{ id_ },
		form{ form_ },
		throwsShadow{ throwsShadow_ }
	{
	}
};



class Entity : public Collidable {
public: 
	Entity(vec2 pos_, float rota_, Collidable collide_) :
		Collidable(collide_),
		Basis(pos_, rota_)
	{}

public:
};