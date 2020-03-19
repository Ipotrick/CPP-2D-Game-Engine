#pragma once

#include <algorithm> 
#include <atomic>

#include "glmath.h"

enum class Form : uint8_t{
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

public:
	Collidable(vec2 size_, Form form_, bool solid_, bool dynamic_, vec2 velocity_ = vec2(0, 0)) :
		Basis{},
		size{ size_ },
		form{ form_ },
		dynamic{ dynamic_ },
		velocity{ velocity_ },
		solid{ solid_ }
	{
	}

	Collidable(Collidable const& c) :
		Basis{c.getPos(), c.getRota()},
		size{ c.getSize() },
		form{ c.getForm() },
		dynamic{ c.isDynamic() },
		velocity{c.velocity},
		solid{ c.isSolid() }
	{
	}

	inline vec2 getVel() const { return velocity; }
	inline Form getForm() const { return form; }
	inline vec2 getSize() const { return size; }
	inline float getRadius() const { assert(form == Form::CIRCLE); return size.r / 2; }
	inline bool isDynamic() const { return dynamic; }
	inline bool isSolid() const { return solid; }

	inline Collidable* getCollidablePtr() { return this; }
	inline Collidable const* getConstCollidablePtr() const { return this; }
	vec2 getBoundsSize() const;
	float getBoundsRadius() const;

public:
	vec2 size;
	vec2 velocity;
	bool solid;
protected:
	bool dynamic;
public:
	Form form;
};

inline vec2 Collidable::getBoundsSize() const {
	if (form == Form::CIRCLE)
	{
		return vec2(getBoundsRadius() * 2);
	}
	else
	{
		return vec2(getBoundsRadius() * 2);
	}
}

inline float Collidable::getBoundsRadius() const {
	if (form == Form::CIRCLE) {
		return size.r / 2.0f;
	}
	else {

		return sqrtf((size.x * size.x + size.y * size.y)) / 2.0f;
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