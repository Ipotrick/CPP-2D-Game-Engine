#pragma once
#include <functional>
#include <boost/container/static_vector.hpp>

#include "JobManager.hpp"
#include "Engine.hpp"


/*
	name is the name of the class that holds the script
	Component is the Component datatype that the script is attached to
*/
#define GAME_SCRIPT(name, Component) \
struct name : public GameScript<Component> { \
using GameScript<Component>::GameScript; \
virtual void script(Entity id, Component& data, float deltaTime) override; \
};

template<typename CompType>
class GameScript {
public:
	GameScript(Engine& engine) : engine{ engine }, world{ engine.world } { }

	/* calls "executeSample" on every component in of the componentController that is given in constructor */
	void execute(float deltaTime);
	/* calls "executeSample" in jobs */
	template<int JobSize>
	void execute(float deltaTime, JobManager& jobManager);
	virtual void script(Entity, CompType&, float) = 0;
protected:
	friend class ScriptJob;
	Engine& engine;
	World& world;
};

template<typename CompType>
inline void GameScript<CompType>::execute(float deltaTime) {

	for (auto entity : engine.world.entityView<CompType>()) {
		script(entity, engine.world.getComp<CompType>(entity), deltaTime);
	}
}

template<typename CompType>
template<int JobSize>
inline void GameScript<CompType>::execute(float deltaTime, JobManager& jobManager)
{
	// dispatch jobs:
	class ScriptJob : public JobFunctor {
	public:
		ScriptJob(GameScript& gameScript, float deltaTime) : gameScript{ gameScript }, deltaTime{ deltaTime }{}
		GameScript& gameScript;
		const float deltaTime;
		std::array<Entity, JobSize> entities;
		size_t size{ 0 };
		void execute(int workerId)
		{
			for (int i = 0; i < size; ++i) {
				Entity entity = entities[i];
				gameScript.script(entity, gameScript.engine.world.getComp<CompType>(entity), deltaTime);
			}
		}
	};

	std::vector<ScriptJob> jobs;
	std::vector<Tag> jobTags;

	jobs.push_back(ScriptJob(*this, deltaTime));
	for (auto entity : engine.world.entityView<CompType>()) {
		if (jobs.back().size == JobSize) {
			jobs.push_back(ScriptJob(*this, deltaTime));
		}
		jobs.back().entities[jobs.back().size] = entity;
		jobs.back().size += 1;
	}

	for (auto& job : jobs) {
		jobTags.push_back(jobManager.addJob(&job));
	}
	jobManager.waitAndHelp(&jobTags);
}