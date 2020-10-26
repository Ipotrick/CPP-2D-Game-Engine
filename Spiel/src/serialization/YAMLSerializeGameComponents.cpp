#include "YAMLSerializeGameComponents.hpp"

YAML::Emitter& operator<<(YAML::Emitter& out, const Player& v)
{
	out << YAML::BeginMap;

	out << YAML::Key << "BullletShotFrequency" << YAML::Value << v.bulletShotLapTimer;
	out << YAML::Key << "FireSpwanFrequency" << YAML::Value << v.flameSpawnTimer;
	out << YAML::Key << "Power" << YAML::Value << v.power;

	out << YAML::EndMap;
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const ParticleScriptComp& v)
{
	out << YAML::BeginMap;

	out << YAML::Key << "StartSize" << YAML::Value << v.startSize;
	out << YAML::Key << "EndSize" << YAML::Value << v.endSize;
	out << YAML::Key << "StartColor" << YAML::Value << v.startColor;
	out << YAML::Key << "EndColor" << YAML::Value << v.endColor;
	out << YAML::Key << "CollisionCount" << YAML::Value << v.collisionCount;

	out << YAML::EndMap;
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const Health& v)
{
	out << YAML::BeginMap;

	out << YAML::Key << "maxHealth" << YAML::Value << v.maxHealth;
	out << YAML::Key << "curHealth" << YAML::Value << v.curHealth;

	out << YAML::EndMap;
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const Age& v)
{
	out << YAML::BeginMap;

	out << YAML::Key << "maxAge" << YAML::Value << v.maxAge;
	out << YAML::Key << "curAge" << YAML::Value << v.curAge;

	out << YAML::EndMap;
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const Bullet& v)
{
	out << YAML::BeginMap;

	out << YAML::Key << "damage" << YAML::Value << v.damage;
	out << YAML::Key << "hitPoints" << YAML::Value << v.hitPoints;

	out << YAML::EndMap;
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const Tester& v)
{
	out << YAML::BeginMap;

	out << YAML::Key << "changeDirTime" << YAML::Value << v.changeDirTime;

	out << YAML::EndMap;
	return out;
}
