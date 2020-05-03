#pragma once

#include <vector>

#include "robin_hood.h"

#include "vector_math.h"
#include "collision_detection.h"
#include "Physics.h"
#include "CoreSystem.h"
#include "PhysicsWorker.h"
#include "Perf.h"

class PhysicsSystem : public CoreSystem {
public:
	PhysicsSystem(World& world, uint32_t threadCount, PerfLogger& perfLog, float statCollGridRes = 0.5f, uint32_t qtreeCapacity = 6);
	void execute(float deltaTime);
	std::tuple<std::vector<CollisionInfo>::iterator, std::vector<CollisionInfo>::iterator> getCollisions(entity_handle id_);
	GridPhysics<bool> getStaticGrid();
	void end();
public:
	std::vector<Drawable> debugDrawables;
private:
	void prepare();
	void collisionDetection();
	void applyPhysics(float deltaTime);
	template<int N> void syncCompositPhysics();
private:
	PerfLogger& perfLog;
	uint32_t const threadCount;
	uint32_t qtreeCapacity;
	bool rebuildStaticData;

	std::vector<std::thread> workerThreads;
	std::shared_ptr<PhysicsSharedSyncData> syncWorkerData;
	std::shared_ptr<PhysicsPoolData> poolWorkerData;
	std::vector<std::shared_ptr<PhysicsPerThreadData>> perWorkerData;

	// buffers
	std::vector<Vec2> oldPosCache;
	std::vector<std::vector<CollisionInfo>> collisionInfosSplit;
	std::vector<CollisionInfo> collisionInfos{};
	robin_hood::unordered_map<entity_handle, std::vector<CollisionInfo>::iterator> collisionInfoBegins;
	robin_hood::unordered_map<entity_handle, std::vector<CollisionInfo>::iterator> collisionInfoEnds;
};