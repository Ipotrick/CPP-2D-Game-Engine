#include "CollisionSystem.hpp"

CollisionSystem::CollisionSystem(CollisionSECM secm, uint32_t qtreeCapacity) :
	secm{ secm },
	qtreeCapacity{ qtreeCapacity },
	qtreeDynamic({ 0,0 }, { 0,0 }, qtreeCapacity, secm, Collider::DYNAMIC),
	qtreeStatic({ 0,0 }, { 0,0 }, qtreeCapacity, secm, Collider::STATIC),
	qtreeParticle({ 0,0 }, { 0,0 }, qtreeCapacity, secm, Collider::PARTICLE),
	qtreeSensor({ 0,0 }, { 0,0 }, qtreeCapacity, secm, Collider::SENSOR)
{
	jobEntityBuffers.push_back(std::make_unique<std::vector<EntityHandleIndex>>());

	for (int i = 0; i < JobSystem::workerCount(); i++) {
		collisionLists.push_back(std::vector<CollisionInfo>());
	}
}

void CollisionSystem::execute(CollisionSECM secm, float deltaTime)
{
	prepare(secm);
	collisionDetection(secm);
}

std::vector<std::vector<CollisionInfo>>& CollisionSystem::getCollisionsLists()
{
	return collisionLists;
}

const CollisionSystem::CollisionsView CollisionSystem::collisions_view(EntityHandleIndex entity)
{
	if (auto* token = secm.getIf<CollisionsToken>(entity)) {
		if (token->workerIndex != -1) {
			return CollisionsView(token->begin, token->end, collisionLists[token->workerIndex]);
		}
	}
	return CollisionsView(0, 0, dummy);
}

const std::vector<Sprite>& CollisionSystem::getDebugSprites() const
{
	return debugSprites;
}

void CollisionSystem::checkForCollisions(std::vector<CollisionInfo>& collisions, uint8_t colliderType, Transform const& b, Collider const& c) const
{
	Vec2 aabb = aabbBounds(c.size, b.rotaVec);
	std::vector<EntityHandleIndex> near;
	std::vector<QtreeNodeQuerry> buffer;
	if (colliderType & Collider::DYNAMIC) {
		qtreeDynamic.querry(near, buffer, b.position, aabb);
	}
	if (colliderType & Collider::STATIC) {
		qtreeStatic.querry(near, buffer, b.position, aabb);
	}
	if (colliderType & Collider::PARTICLE) {
		qtreeParticle.querry(near, buffer, b.position, aabb);
	}
	if (colliderType & Collider::SENSOR) {
		qtreeSensor.querry(near, buffer, b.position, aabb);
	}
	std::vector<CollPoint> verteciesBuffer;
	generateCollisionInfos2(secm, collisions, aabbCache, near, INVALID_ENTITY_HANDLE_INDEX, b, c, aabb, verteciesBuffer);
}

inline size_t CollisionSystem::collisionCount() const
{
	size_t acc = 0;
	for (auto& c : collisionLists) acc += c.size();
	return acc;
}

void CollisionSystem::prepare(CollisionSECM secm)
{
	cleanBuffers(secm);

	JobSystem::Tag clearCollTokensJobTag = JobSystem::submit(
		LambdaJob{
			[&](u32 thread) {
				clearCollisionTokens();
			}
		}
	);

	Vec2 minPos{ 0,0 }, maxPos{ 0,0 };
	for (auto colliderEnt : secm.entityView<Collider>()) {
		auto colliderID = colliderEnt.index;
		auto& collider = secm.getComp<Collider>(colliderID);
		auto& baseCollider = secm.getComp<Transform>(colliderID);
		minPos = min(minPos, baseCollider.position);
		maxPos = max(maxPos, baseCollider.position);

		if (secm.hasComp<PhysicsBody>(colliderID)) { // if a collider has a solidBody, it is a physics object
			if (secm.hasComp<Movement>(colliderID)) {	// is it dynamic or static?
				if (collider.particle) {
					particleEntities.push_back(colliderID);
				}
				else {
					dynamicSolidEntities.push_back(colliderID);
				}
			}
			else {	// entity must be static
				staticSolidEntities.push_back(colliderID);
			}
		}
		else { // if a collider has NO PhysicsBody, it is a sensor
			sensorEntities.push_back(colliderID);
		}
	}

	JobSystem::wait(JobSystem::submitVec(
		std::vector<CacheAABBJob>{
		CacheAABBJob(particleEntities, secm, aabbCache),
			CacheAABBJob(dynamicSolidEntities, secm, aabbCache),
			CacheAABBJob(staticSolidEntities, secm, aabbCache),
			CacheAABBJob(sensorEntities, secm, aabbCache)
	}
	));

	/* rebuild qtrees: */

	qtreeDynamic.resetPerMinMax(minPos, maxPos);
	qtreeDynamic.removeEmptyLeafes();

	if (colliderDetectionEnableFlags & qtreeDynamic.COLLIDER_TAG) {
		qtreeDynamic.broadInsert(dynamicSolidEntities, aabbCache);
	}

	qtreeStatic.resetPerMinMax(minPos, maxPos);
	qtreeStatic.removeEmptyLeafes();

	if (colliderDetectionEnableFlags & qtreeStatic.COLLIDER_TAG) {
		qtreeStatic.broadInsert(staticSolidEntities, aabbCache);
	}

	qtreeParticle.resetPerMinMax(minPos, maxPos);
	qtreeParticle.removeEmptyLeafes();

	if (colliderDetectionEnableFlags & qtreeParticle.COLLIDER_TAG) {
		qtreeParticle.broadInsert(particleEntities, aabbCache);
	}

	qtreeSensor.resetPerMinMax(minPos, maxPos);
	qtreeSensor.removeEmptyLeafes();

	if (colliderDetectionEnableFlags & qtreeSensor.COLLIDER_TAG) {
		qtreeSensor.broadInsert(sensorEntities, aabbCache);
	}

	JobSystem::wait(clearCollTokensJobTag);
}

void CollisionSystem::cleanBuffers(CollisionSECM secm)
{
	debugSprites.clear();
	particleEntities.clear();
	sensorEntities.clear();
	dynamicSolidEntities.clear();
	staticSolidEntities.clear();
	aabbCache.clear();
	if (aabbCache.size() < secm.maxEntityIndex()) {
		aabbCache.resize(secm.maxEntityIndex());
	}
	for (auto& collisionList : collisionLists) {
		collisionList.clear();
	}
	for (auto& jobBuffer : jobEntityBuffers) {
		jobBuffer->clear();
	}
}

void CollisionSystem::collisionDetection(CollisionSECM secm)
{
	class CollJob : public IJob {
	public:

		CollJob(
			CollisionSECM subecm,
			StaticVector<Quadtree const*, 4> qtrees,
			std::vector<Vec2> const* aabbCache,
			std::vector<std::vector<CollisionInfo>>* collInfos)
			:
			subecm{ subecm },
			qtrees{ qtrees },
			aabbCache{ aabbCache },
			collInfos{ collInfos }
		{}

		void execute(const uint32_t thread) override
		{
			auto checkForCollisions = [&](EntityHandleIndex ent, Quadtree const& qtree) {
				const auto& baseColl = subecm.getComp<Transform>(ent);
				const auto& colliderColl = subecm.getComp<Collider>(ent);

				nearEntitiesBuffer.clear();
				qtreeQuerryBuffer.clear();
				collPoints.clear();

				//if (!colliderColl.sleeping) {
					qtree.querry(nearEntitiesBuffer, qtreeQuerryBuffer, baseColl.position, aabbCache->at(ent));

					generateCollisionInfos2(subecm, collInfos->at(thread), *aabbCache, nearEntitiesBuffer, ent, baseColl, colliderColl, aabbCache->at(ent), collPoints);
				//}
			};

			for (int i = 0; i < entities.size(); ++i) {
				EntityHandleIndex ent = entities[i];

				Collider const& entColliderComp = subecm.getComp<Collider>(ent);

				for (int j = 0; j < qtrees.size(); ++j) {
					Quadtree const* qtree = qtrees[j];

					if (!entColliderComp.isIgnoring(qtree->COLLIDER_TAG)) {
						checkForCollisions(ent, *qtree);
					}
				}
			}
		}

		StaticVector<EntityHandleIndex, MAX_ENTITIES_PER_JOB> entities;
	private:
		std::vector<std::vector<CollisionInfo>>* collInfos;
		StaticVector<Quadtree const*, 4> qtrees;
		CollisionSECM subecm;
		std::vector<Vec2> const* aabbCache;

		// buffers for queriing:
		std::vector<EntityHandleIndex> nearEntitiesBuffer;
		std::vector<QtreeNodeQuerry> qtreeQuerryBuffer;
		std::vector<CollPoint> collPoints;
	};

	std::vector<CollJob> jobs;
	jobs.reserve(200);

	auto createCollisionCheckJobs = [&](const std::vector<EntityHandleIndex>& entities, const uint8_t qtreeMask) {

		StaticVector<Quadtree const*, 4> qtrees;
		if (qtreeDynamic.COLLIDER_TAG & qtreeMask) { qtrees.push_back(&qtreeDynamic); }
		if (qtreeStatic.COLLIDER_TAG & qtreeMask) { qtrees.push_back(&qtreeStatic); }
		if (qtreeParticle.COLLIDER_TAG & qtreeMask) { qtrees.push_back(&qtreeParticle); }
		if (qtreeSensor.COLLIDER_TAG & qtreeMask) { qtrees.push_back(&qtreeSensor); }

		const auto newCollJob = CollJob(
			secm,
			qtrees,
			&aabbCache,
			&collisionLists
		);

		auto c = newCollJob;

		// cut up entities vector into smaller parts (MAX SIZE = MAX_ENTITIES_PER_JOB) 
		// and save them i the jobs vector
		for (auto el : entities) {
			c.entities.push_back(el);
			if (c.entities.size() == MAX_ENTITIES_PER_JOB) {
				jobs.push_back(c);
				c = newCollJob;
			}
		}
		// dont forget the last job that potentially only has less entities than MAX_ENTITIES_PER_JOB!:
		if (c.entities.size() > 0) {
			jobs.push_back(c);
		}
	};

	createCollisionCheckJobs(particleEntities, Collider::DYNAMIC | Collider::STATIC);

	createCollisionCheckJobs(dynamicSolidEntities, Collider::DYNAMIC | Collider::STATIC);

	createCollisionCheckJobs(staticSolidEntities, Collider::DYNAMIC);

	createCollisionCheckJobs(sensorEntities, Collider::PARTICLE | Collider::DYNAMIC | Collider::SENSOR | Collider::STATIC);

	auto tag = JobSystem::submitVec(std::move(jobs));

	JobSystem::wait(tag);

	// reset quadtree rebuild flags
	rebuildStatic = false;

	for (u32 wi = 0; wi < collisionLists.size(); wi++) {
		auto& workerCollInfos = collisionLists[wi];
		if (workerCollInfos.size() > 0) {
			EntityHandleIndex currentEntity = workerCollInfos[0].indexA;
			secm.getComp<CollisionsToken>(currentEntity).begin = 0;
			secm.getComp<CollisionsToken>(currentEntity).workerIndex = wi;
			for (int i = 1; i < workerCollInfos.size(); i++) {
				if (currentEntity != workerCollInfos[i].indexA) {	//new idA found
					auto nextEntity = workerCollInfos[i].indexA;
					secm.getComp<CollisionsToken>(nextEntity).begin = i;
					secm.getComp<CollisionsToken>(nextEntity).workerIndex = wi;
					secm.getComp<CollisionsToken>(currentEntity).end = i;
					currentEntity = nextEntity;	//set lastId to new id
				}
			}
			secm.getComp<CollisionsToken>(currentEntity).end = uint32_t(workerCollInfos.size());
		}
	}
}

void CollisionSystem::clearCollisionTokens()
{
	for (EntityHandle entity : secm.entityView<Collider>()) {
		if (auto* token = secm.getIf<CollisionsToken>(entity.index)) {
			token->begin = 0;
			token->end = 0;
			token->workerIndex = -1;
		}
		else {
			secm.addComp<CollisionsToken>(entity);
		}
	}
}
