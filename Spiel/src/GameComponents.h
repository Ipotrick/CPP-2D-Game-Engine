#pragma once

#include "glmath.h"
#include "Component.h"

// drawable component

struct CompDataDrawable : public CompData {
	enum class Form {
		CIRCLE,
		RECTANGLE
	};
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

// mortal component

struct CompDataMortal : public CompData {
	CompDataMortal( int maxHealth_, int collisionDamage_, float maxAge_) :
		maxHealth{ maxHealth_ },
		curHealth{ maxHealth },
		collisionDamage{ collisionDamage_ },
		maxAge{ maxAge_ },
		curAge{ 0.0f }
	{}
	//(maxHealth <= 0) => ignore health
	int maxHealth;
	int curHealth;
	//(maxAge < 0) => ignore age
	float maxAge;
	float curAge;
	int collisionDamage;
};

//player component

struct CompDataPlayer : public CompData {
};

//bullet component

struct CompDataBullet : CompData {
};