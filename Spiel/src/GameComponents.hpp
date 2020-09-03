#pragma once

#include "Timing.hpp"
#include "BaseTypes.hpp"

#include "EntityComponentStorage.hpp"

struct SpawnerComp : public CompData {
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version)
	{
		ar& sucker;
	}
	EntityId sucker;
};

struct SuckerComp : public CompData {
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version)
	{
		ar& spawner;
	}
	EntityId spawner;
};

struct ParticleScriptComp : public CompData {
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version)
	{
		ar& startSize;
		ar& endSize;
		ar& startColor;
		ar& endColor;
	}
	ParticleScriptComp() {}
	ParticleScriptComp(Vec2 startSize, Vec2 endSize, Vec4 startColor, Vec4 endColor) : startSize{ startSize }, endSize{ endSize }, startColor{ startColor }, endColor{ endColor } {}
	Vec2 startSize;
	Vec2 endSize;
	Vec4 startColor;
	Vec4 endColor;
};

// player component

struct Player : public CompData {
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version)
	{
		ar& bulletShotLapTimer;
		ar& flameSpawnTimer;
		ar& dummyExis;
		ar& power;
	}
	Player() : bulletShotLapTimer{ 0.01f }, flameSpawnTimer{ 0.001f }, dummyExis{ EntityId() } {}
	LapTimer<> bulletShotLapTimer;
	LapTimer<> flameSpawnTimer;
	EntityId dummyExis;
	float power{ 1.0f };
};

// health component

struct Health : public CompData {
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version)
	{
		ar& maxHealth;
		ar& curHealth;
	}
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
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version)
	{
		ar& maxAge;
		ar& curAge;
	}
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
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version)
	{
		ar& damage;
		ar& hitPoints;
	}
	Bullet(int damage_, int hitPoints) :damage{ damage_ }, hitPoints{ hitPoints } {}
	Bullet() : damage{ 0 } , hitPoints{1} {}
	int damage;
	int hitPoints;
};

// loading and event trigger component

struct Trigger : public CompData {
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version)
	{
		ar& type;
		ar& mapname;
	}
	Trigger(int type_, std::string mapname_) :
		type{ type_ },
		mapname{mapname_}
	{}

	int type;
	std::string mapname;
};

// enemy component

struct Enemy : public CompData {
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version)
	{
		ar& target;
	}
	Enemy(Entity tar_ = 0) : target{ tar_} {}
	Entity target;
};

// dummy component

struct Dummy : public CompData {
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version)
	{
		ar& player_id;
	}
	Dummy(EntityId player_id_ = EntityId()) : player_id{ player_id_} {}
	EntityId player_id;
};

struct Tester {
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version)
	{
		ar& changeDirTime;
	}
	float changeDirTime = 0.0f;
};
