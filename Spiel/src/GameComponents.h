#pragma once

#include "glmath.h"
#include "Component.h"
#include "Timing.h"
#include "Entity.h"

//player component

struct CompDataPlayer : public CompData {
	CompDataPlayer() : bulletShotLapTimer{ 0.001f } {}
	LapTimer<> bulletShotLapTimer;
};

// health component

struct CompDataHealth : public CompData {
	CompDataHealth(int maxHealth_) :
		maxHealth{ maxHealth_ },
		curHealth{ maxHealth_ }
	{}
	int maxHealth;
	int curHealth;
};

// age component

struct CompDataAge : public CompData {
	CompDataAge(float maxAge_) :
		maxAge{ maxAge_ },
		curAge{ 0.0f }
	{}
	float maxAge;
	float curAge;
};

//bullet component

struct CompDataBullet : public CompData {
	CompDataBullet(int damage_) :damage{ damage_ } {}

	int damage;
	
};

//loading and event trigger component
//0 = inactive, 1 = event trigger, 2, loading trigger

struct CompDataTrigger : public CompData {
	CompDataTrigger(int type_) :type{ type_ } {}

	int type;

};

//owner component

struct CompDataOwner : public CompData {
	CompDataOwner(uint32_t slave_) :slave{ slave_ } {}

	uint32_t slave;
};

//slave component

struct CompDataSlave : public CompData {
	CompDataSlave(uint32_t owner_) :owner{ owner_ } {}

	uint32_t owner;

};

// light component

struct CompDataLight : public CompData {
	CompDataLight(vec4 col_ = vec4(1,1,1,1)) :color{col_} {}
	vec4 color;
};