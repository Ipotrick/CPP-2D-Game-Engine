#pragma once

#include "Collidable.h"
#include "Drawable.h"

class Entity : virtual public Drawable, virtual public Collidable {
public: 
	Entity(Drawable&& draw_, Collidable&& collide_) : Collidable(collide_), Drawable(draw_)
	{

	}
};