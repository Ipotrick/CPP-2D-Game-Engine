#pragma once

#include "Collidable.h"
#include "Drawable.h"

class Entity : public Collidable, public Drawable {
public: 
	Entity(Drawable draw_, Collidable collide_) :
		Collidable(collide_),
		Drawable(draw_),
		Basis(draw_.position, draw_.rotation),
		health{100},
		despawned{false}
	{}

	Entity():
		Collidable(),
		Drawable(),
		Basis(),
		health{100},
		despawned{ false }
	{}
public:
	int health;
	bool despawned;

};