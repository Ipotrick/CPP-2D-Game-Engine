#pragma once

#include "glmath.h"
#include "Component.h"
#include "Timing.h"
#include "Entity.h"

//player component

struct Player : public CompData {
	Player() : bulletShotLapTimer{ 0.001f } {}
	LapTimer<> bulletShotLapTimer;
};

// health component

struct Health : public CompData {
	Health(int maxHealth_) :
		maxHealth{ maxHealth_ },
		curHealth{ maxHealth_ }
	{}

	Health() : maxHealth{0}, curHealth{0} {}
	int maxHealth;
	int curHealth;
};

// age component

struct Age : public CompData {
	Age(float maxAge_) :
		maxAge{ maxAge_ },
		curAge{ 0.0f }
	{}
	Age() : maxAge{0}, curAge{0} {}
	float maxAge;
	float curAge;
};

//bullet component

struct Bullet : public CompData {
	Bullet(int damage_) :damage{ damage_ } {}
	Bullet() : damage {0} {}
	int damage;
	
};

//loading and event trigger component

struct Trigger : public CompData {
	Trigger(int type_) :type{ type_ } {}

	int type;

};

// light component

struct CompDataLight : public CompData {
	CompDataLight(vec4 col_ = vec4(1,1,1,1)) :color{col_} {}
	vec4 color;
};