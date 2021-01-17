#pragma once

#include "../engine/types/Timing.hpp"
#include "../engine/types/BaseTypes.hpp"
#include "../engine/types/UUID.hpp"

#include "../engine/entity/EntityComponentStorage.hpp"
#include "../engine/EngineCore.hpp"

struct SpawnerComp {
};

struct SuckerComp {
	UUID spawner;
};

struct ParticleScriptComp {
	ParticleScriptComp() = default;
	ParticleScriptComp(Vec2 startSize, Vec2 endSize, Vec4 startColor, Vec4 endColor) : startSize{ startSize }, endSize{ endSize }, startColor{ startColor }, endColor{ endColor } {}
	Vec2 startSize;
	Vec2 endSize;
	Vec4 startColor;
	Vec4 endColor;
	int collisionCount = 0;
};

// player component

struct Player {
	LapTimer bulletShotLapTimer{ 0.005f };
	LapTimer flameSpawnTimer{ 0.008f };
	float power{ 1.0f };
};

// health component

struct Health {
	Health(int maxHealth_) :
		maxHealth{ maxHealth_ },
		curHealth{ maxHealth_ }
	{}

	Health() : maxHealth{ 0 }, curHealth{ 0 } {}

	int maxHealth;
	int curHealth;
	UIEntityHandle healthBar = {};
};

// age component

struct Age {
	Age(float maxAge_) :
		maxAge{ maxAge_ },
		curAge{ 0.0f }
	{}
	Age() : maxAge{0}, curAge{0} {}
	float maxAge;
	float curAge;
};

// bullet component

struct Bullet {
	Bullet(int damage_, int hitPoints) :damage{ damage_ }, hitPoints{ hitPoints } {}
	Bullet() : damage{ 0 } , hitPoints{1} {}
	int damage;
	int hitPoints;
};

// loading and event trigger component

struct Trigger {
	Trigger(int type_, std::string mapname_) :
		type{ type_ },
		mapname{mapname_}
	{}

	int type;
	std::string mapname;
};

// enemy component

struct Enemy {
	Enemy(EntityHandle tar_ = EntityHandle()) : target{ tar_} {}
	EntityHandle target;
};

// dummy component

struct Dummy {
	Dummy(EntityHandle player_id_ = EntityHandle()) : player_id{ player_id_} {}
	EntityHandle player_id;
};

struct Tester {
	float changeDirTime = 0.0f;
};
