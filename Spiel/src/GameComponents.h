#pragma once

#include "CompController.h"
#include "Engine.h"

template<typename CompDataType>
class GameCompController : public BasicCompController<CompDataType> {
public:
	GameCompController(Engine& engine_, std::vector<void*>& otherComponents_) : BasicCompController<CompDataType>{}, engine { engine_ }, otherComponents{ otherComponents_ }
	{}


protected:
	Engine& engine;
	std::vector<void*>& otherComponents;
};

// mortal component index: 0

struct CompDataMortal : public CompData {
	CompDataMortal(uint32_t id_, int maxHealth_, int collisionDamage_, float maxAge_) :
		CompData{ id_ },
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

class CompControllerMortal : public GameCompController<CompDataMortal> {
public:
	CompControllerMortal(Engine& engine, std::vector<void*>& otherComponents_) : GameCompController<CompDataMortal>{ engine, otherComponents_ } {}
protected:
	void executeScript(CompDataMortal& data, float deltaTime) override;
};

//player component index: 1

struct CompDataPlayer : public CompData {
	CompDataPlayer(uint32_t id_) :
		CompData{ id_ }
	{}
};

class CompControllerPlayer : public GameCompController<CompDataPlayer> {
public:
	CompControllerPlayer(Engine& engine, std::vector<void*>& otherComponents_) : GameCompController<CompDataPlayer>{ engine, otherComponents_ } {}
protected:
	void executeScript(CompDataPlayer& data, float deltaTime) override;
};

struct CompDataBullet : CompData {
	CompDataBullet(uint32_t id_) :
		CompData{ id_ }
	{}
};

//bullet component index: 2

class CompControllerBullet : public GameCompController<CompDataBullet> {
public:
	CompControllerBullet(Engine& engine, std::vector<void*>& otherComponents_) : GameCompController<CompDataBullet>{ engine, otherComponents_ } {}
protected:
	void executeScript(CompDataBullet& data, float deltaTime) override;
};
