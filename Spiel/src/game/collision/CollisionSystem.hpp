#pragma once

#include <vector>

#include <boost/container/static_vector.hpp>
#include <robin_hood.h>

#include "CollisionUniform.hpp"
#include "../../engine/entity/EntityComponentManagerView.hpp"
#include "../../engine/math/vector_math.hpp"
#include "../collision/collision_detection.hpp"
#include "QuadTree.hpp"
#include "CacheAABBJob.hpp"
#include "../../engine/types/StaticVector.hpp"

#define DEBUG_QTREE_FINDPLAYER

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
	CollisionSystem(CollisionSECM secm, uint32_t qtreeCapacity = 6);
	void execute(CollisionSECM secm, float deltaTime);
	std::vector<CollisionInfo>& getCollisions();
	const CollisionsView collisions_view(EntityHandle entity)
	{
		return collisions_view(entity.index);
	}
	const CollisionsView collisions_view(EntityHandleIndex entity);
	const std::vector<Drawable>& getDebugDrawables() const;
	void checkForCollisions(std::vector<CollisionInfo>& collisions, uint8_t colliderType, Transform const& b, Collider const& c) const;

	void disableColliderDetection(uint8_t qtreeIgnoreFlags)
	{
		colliderDetectionEnableFlags &= ~qtreeIgnoreFlags;
	}

	void enableColliderDetection(uint8_t qtreeEnableFrags)
	{
		colliderDetectionEnableFlags |= qtreeEnableFrags;
	}
private:
	void prepare(CollisionSECM secm);
	void cleanBuffers(CollisionSECM secm);
	void collisionDetection(CollisionSECM secm);

	std::vector<Drawable> debugDrawables;

	CollisionSECM secm;
	// constants:
	static const int MAX_ENTITIES_PER_JOB = 200;
	uint32_t qtreeCapacity;
	bool rebuildStaticData;
	// flags:
	bool rebuildStatic = true;
	// buffers
	Quadtree qtreeDynamic;
	Quadtree qtreeStatic;
	Quadtree qtreeParticle;
	Quadtree qtreeSensor;
	uint8_t colliderDetectionEnableFlags{ 0xFF };

	std::vector<Vec2> aabbCache;

	std::vector<EntityHandleIndex> sensorEntities;
	std::vector<EntityHandleIndex> particleEntities;
	std::vector<EntityHandleIndex> dynamicSolidEntities;
	std::vector<EntityHandleIndex> staticSolidEntities;
	std::vector<std::vector<CollisionInfo>> collInfoPerWorker;

	std::vector<std::unique_ptr<std::vector<EntityHandleIndex>>> jobEntityBuffers;

	std::vector<CollisionInfo> collisionInfos{};
};
