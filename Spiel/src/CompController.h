#pragma once

#include "Engine.h"

struct CompData {
	CompData(uint32_t id_) : id{id_} {}
	uint32_t id;
};

template<typename CompDataType>
class CompController {
public:
	CompController(Engine& engine_, std::vector<void*>& otherComponents_) : engine{ engine_ }, otherComponents{ otherComponents_ }, componentData{}
	{}

	/* register an entitiy to be affected by the component. The parameter is the constructor parameter for the component data, O(1) !MAY CAUSE A REALLOCATION! */
	void registerEntity(CompDataType const& componentData);
	/* remove entitiy from the infruence of the component, O(n) */
	void deregisterEntity(uint32_t id);
	/* returns true if an entity with the given id is registered in the ComponentController, O(log2(n)) */
	bool isRegistered(uint32_t);
	/* returns a pointer to the componentData of the given entity. If the given id is not registered nullptr will be returned, O(log2(n)) */
	CompDataType const * const getComponent(uint32_t id);
	/* executes the component script on all registered componentes, O(n)*/
	void executeScripts(float deltaTime);

protected:
	/* script for one component, O(1) */
	/* try to NOT querry the entity when ever possible (so once or not at all per call), as the entity querry has a runtime of O(log2(m))! (m is the count of entities) */
	virtual void executeScript(CompDataType& data, float deltaTime) = 0;

protected:
	Engine & engine;
	std::vector<void*>& otherComponents;
	std::vector<CompDataType> componentData;
};

template<typename CompDataType>
inline void CompController<CompDataType>::registerEntity(CompDataType const& newComponentData)
{
	componentData.push_back(newComponentData);
}

template<typename ComponentDataType>
void CompController<ComponentDataType>::deregisterEntity(uint32_t id)
{
	for (auto iter = componentData.begin(); iter != componentData.end(); ++iter) {
		if (iter->id == id) {
			componentData.erase(iter);
			break;
		}
	}
}

template<typename CompDataType>
inline bool CompController<CompDataType>::isRegistered(uint32_t id)
{
	auto iter = std::lower_bound(componentData.begin(), componentData.end(), id,
		[](CompDataType const& a, uint32_t b)
		{
			return a.id < b;
		}
	);
	if (iter != componentData.end() && iter->id == id) {
		return true;
	}
	else {
		return false;
	}
}

template<typename CompDataType>
inline CompDataType const* const CompController<CompDataType>::getComponent(uint32_t id)
{
	auto iter = std::lower_bound(componentData.begin(), componentData.end(), id,
		[](CompDataType const& a, uint32_t b)
		{
			return a.id < b;
		}
	);
	if (iter != componentData.end() && iter->id == id) {
		return &*iter;
	}
	else {
		return nullptr;
	}
}

template<typename CompDataType>
inline void CompController<CompDataType>::executeScripts(float deltaTime)
{
	for (auto & el : componentData) {
		executeScript(el, deltaTime);
	}
}

// mortal component index: 0

struct CompDataMortal : public CompData {
	CompDataMortal(uint32_t id_, int maxHealth_, int collisionDamage_, float maxAge_):
		CompData{id_},
		maxHealth{ maxHealth_},
		curHealth{maxHealth},
		collisionDamage{ collisionDamage_ },
		maxAge{ maxAge_},
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

class CompControllerMortal : public CompController<CompDataMortal>{
public:
	CompControllerMortal(Engine& engine, std::vector<void*>& otherComponents_) : CompController<CompDataMortal>{ engine, otherComponents_ } {}
protected:
	void executeScript(CompDataMortal& data, float deltaTime) override;
};

//player component index: 1

struct CompDataPlayer : public CompData {
	CompDataPlayer(uint32_t id_) :
		CompData{ id_ }
	{}
};

class CompControllerPlayer : public CompController<CompDataPlayer> {
public:
	CompControllerPlayer(Engine& engine, std::vector<void*>& otherComponents_) : CompController<CompDataPlayer>{ engine, otherComponents_ } {}
protected:
	void executeScript(CompDataPlayer& data, float deltaTime) override;
};

struct CompDataBullet : CompData {
	CompDataBullet(uint32_t id_, int maxHealth_, int collisionDamage_, int selfCollisionDamage_, float maxAge_) :
		CompData{ id_ },
		maxHealth { maxHealth_ },
		curHealth { maxHealth },
		collisionDamage { collisionDamage_ },
		selfCollisionDamage { selfCollisionDamage_ },
		maxAge { maxAge_ },
		curAge { 0.0f }
	{}
	//(maxHealth <= 0) => ignore health
	int maxHealth;
	int curHealth;
	//(maxAge < 0) => ignore age
	float maxAge;
	float curAge;
	int collisionDamage;
	int selfCollisionDamage;
};

class CompControllerBullet : public CompController<CompDataBullet> {
public:
	CompControllerBullet(Engine& engine, std::vector<void*>& otherComponents_) : CompController<CompDataBullet>{ engine, otherComponents_ } {}
protected:
	void executeScript(CompDataBullet& data, float deltaTime) override;
};
