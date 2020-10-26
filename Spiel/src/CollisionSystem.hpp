#pragma once

#include <vector>

#include "robin_hood.h"

#include "vector_math.hpp"
#include "collision_detection.hpp"
#include "Physics.hpp"
#include "CoreSystem.hpp"
#include "Perf.hpp"
#include "JobManager.hpp"
#include "CollisionCheckJob.hpp"

class CollisionSystem {
	friend class PhysicsSystem;
	friend class PhysicsSystem2;
	class CollisionsView {
	public:
		explicit CollisionsView(const size_t begin, const size_t end, std::vector<CollisionInfo>& collInfos)
			:beginIndex{ begin }, endIndex{ end }, collInfos{ collInfos }
		{}
		auto begin() const
		{
			return collInfos.begin() + beginIndex;
		}
		auto end() const
		{
			return collInfos.begin() + endIndex;
		}
		CollisionInfo& operator[](size_t index)
		{
			return collInfos[beginIndex + index];
		}
		const CollisionInfo& operator[](size_t index) const
		{
			return collInfos[beginIndex + index];
		}
		CollisionInfo& at(size_t index)
		{
			if (index >= beginIndex && index < endIndex) {
				return collInfos.at(beginIndex + index);
			}
			else {
				throw new std::exception("error: index out of bounds");
			}
		}
		const CollisionInfo& at(size_t index) const 
		{
			if (index >= beginIndex && index < endIndex) {
				return collInfos.at(beginIndex + index);
			}
			else {
				throw new std::exception("error: index out of bounds");
			}
		}
		size_t size() const
		{
			return endIndex - beginIndex;
		}
	private:
		const size_t beginIndex;
		const size_t endIndex;
		std::vector<CollisionInfo>& collInfos;
	};
public:
	CollisionSystem(World& world, JobManager& jobManager, PerfLogger& perfLog, uint32_t qtreeCapacity = 6);
	void execute(World& world, float deltaTime);
	std::vector<CollisionInfo>& getCollisions();
	const CollisionsView collisions_view(EntityHandle entity)
	{
		return collisions_view(entity.index);
	}
	const CollisionsView collisions_view(EntityHandleIndex entity);
	const std::vector<Drawable>& getDebugDrawables() const;
	void checkForCollisions(std::vector<CollisionInfo>& collisions, uint8_t colliderType, Transform const& b, Collider const& c) const;
private:
	void prepare(World& world);
	void cleanBuffers(World& world);
	void collisionDetection(World& world);

	std::vector<Drawable> debugDrawables;

	World& world;
	JobManager& jobManager;
	// constants:
	static const int MAX_ENTITIES_PER_JOB = 200;
	uint32_t qtreeCapacity;
	PerfLogger& perfLog;
	bool rebuildStaticData;
	// flags:
	bool rebuildStatic = true;
	// buffers
	Quadtree qtreeDynamic;
	Quadtree qtreeStatic;
	Quadtree qtreeParticle;
	Quadtree qtreeSensor;

	std::vector<Vec2> aabbCache;

	std::vector<EntityHandleIndex> sensorEntities;
	std::vector<EntityHandleIndex> particleEntities;
	std::vector<EntityHandleIndex> dynamicSolidEntities;
	std::vector<EntityHandleIndex> staticSolidEntities;
	CollisionCheckJobBuffers workerBuffers;

	std::vector<std::unique_ptr<std::vector<EntityHandleIndex>>> jobEntityBuffers;

	std::vector<CollisionInfo> collisionInfos{};
	// buffers for jobs:
	std::vector<CollisionCheckJob> collisionCheckJobs;
	std::vector<Tag> collisionCheckJobTags;
};