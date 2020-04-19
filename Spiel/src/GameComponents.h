#pragma once

#include "Timing.h"
#include "BaseTypes.h"

#include "ECS.h"

// player component

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

// bullet component

struct Bullet : public CompData {
	Bullet(int damage_) :damage{ damage_ } {}
	Bullet() : damage {0} {}
	int damage;
	
};

// loading and event trigger component

struct Trigger : public CompData {
	Trigger(int type_, std::string mapname_) :
		type{ type_ },
		mapname{mapname_}
	{}

	int type;
	std::string mapname;

};

// light component

struct CompDataLight : public CompData {
	CompDataLight(vec4 col_ = vec4(1,1,1,1)) :color{col_} {}
	vec4 color;
};

// enemy component

struct Enemy : public CompData {
	Enemy(ent_id_t tar_ = 0) : target{ tar_} {}
	ent_id_t target;
};