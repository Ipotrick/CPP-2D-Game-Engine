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
		hitboxScale{ size_ },
		hitboxForm{ form_ },
		elasticity{ elasticity_ },
		dynamic{ dynamic_ },
		mass{ mass_ },
		velocity{ velocity_ },
		collided{ false }
	{
		//std::cout << "Collidable Constructor" << std::endl;
		//std::cout << position << std::endl;
	}

	/*Collidable(Collidable const& other) noexcept :
		Basis{},
		id{ nextId++ },
		hitboxScale{ other.hitboxScale },
		hitboxForm{ other.hitboxForm },
		elasticity{ other.elasticity },
		dynamic{ other.dynamic },
		mass{ other.mass },
		velocity{ other.velocity },
		collided{ other.collided }
	{
		//std::cout << "Collidable Constructor" << std::endl;
	}*/



	Form hitboxForm;
	vec2 hitboxScale;
	float mass;
	float elasticity;

	bool collided;
	vec2 velocity;

	inline uint32_t getId() { return id; }
	inline bool isDynamic() { return dynamic; }

	inline Collidable* getCollidablePtr() { return this; }
};