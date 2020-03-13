#pragma once

#include "glmath.h"
#include "Component.h"
#include "Timing.h"
#include "Entity.h"

// drawable component

struct CompDataDrawable : public CompData {
	vec4 color;
	vec2 scale;
	float drawingPrio;
	Form form;

	CompDataDrawable(vec4 color_ = vec4(1,1,1,1), vec2 scale_ = vec2(1,1), float drawingPrio_ = 0.5f, Form form_ = Form::RECTANGLE) :
		color{ color_ },
		scale{ scale_ },
		drawingPrio{ drawingPrio_ },
		form{ form_ }
	{
	}
};

//player component

struct CompDataPlayer : public CompData {
	CompDataPlayer() : bulletShotLapTimer{ 0.01f } {}
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