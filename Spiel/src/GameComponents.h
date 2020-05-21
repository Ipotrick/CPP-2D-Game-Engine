#pragma once

#include "Timing.h"
#include "BaseTypes.h"

#include "EntityComponentStorage.h"

struct ParticleScriptComp : public CompData {
	ParticleScriptComp() {}
	ParticleScriptComp(Vec2 startSize, Vec2 endSize, Vec4 startColor, Vec4 endColor) : startSize{ startSize }, endSize{ endSize }, startColor{ startColor }, endColor{ endColor } {}
	Vec2 startSize;
	Vec2 endSize;
	Vec4 startColor;
	Vec4 endColor;
};

// player component

struct Player : public CompData {
	Player() : bulletShotLapTimer{ 0.01f }, flameSpawnTimer{ 0.001f }, dummyExis{ entity_id() } {}
	LapTimer<> bulletShotLapTimer;
	LapTimer<> flameSpawnTimer;
	entity_id dummyExis;
	float power{ 1.0f };
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
	Bullet(int damage_, int hitPoints) :damage{ damage_ }, hitPoints{ hitPoints } {}
	Bullet() : damage{ 0 } , hitPoints{1} {}
	int damage;
	int hitPoints;
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
	CompDataLight(Vec4 col_ = Vec4(1,1,1,1)) :color{col_} {}
	Vec4 color;
};

// enemy component

struct Enemy : public CompData {
	Enemy(entity_index_type tar_ = 0) : target{ tar_} {}
	entity_index_type target;
};

// dummy component

struct Dummy : public CompData {
	Dummy(entity_id player_id_ = entity_id()) : player_id{ player_id_} {}
	entity_id player_id;
};