#pragma once

#include "Timing.hpp"
#include "BaseTypes.hpp"

#include "EntityComponentStorage.hpp"

struct SpawnerComp : public CompData {
	EntityHandle sucker;
};

struct SuckerComp : public CompData {
	EntityHandle spawner;
};

struct ParticleScriptComp : public CompData {
	ParticleScriptComp() {}
	ParticleScriptComp(Vec2 startSize, Vec2 endSize, Vec4 startColor, Vec4 endColor) : startSize{ startSize }, endSize{ endSize }, startColor{ startColor }, endColor{ endColor } {}
	Vec2 startSize;
	Vec2 endSize;
	Vec4 startColor;
	Vec4 endColor;
	int collisionCount = 0;
};

// player component

struct Player : public CompData {
	Player() : bulletShotLapTimer{ 0.01f }, flameSpawnTimer{ 0.008f } {}
	LapTimer<> bulletShotLapTimer;
	LapTimer<> flameSpawnTimer;
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
	bool bUISpawned{ false };
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

// enemy component

struct Enemy : public CompData {
	Enemy(EntityHandle tar_ = EntityHandle()) : target{ tar_} {}
	EntityHandle target;
};

// dummy component

struct Dummy : public CompData {
	Dummy(EntityHandle player_id_ = EntityHandle()) : player_id{ player_id_} {}
	EntityHandle player_id;
};

struct Tester {
	float changeDirTime = 0.0f;
};
