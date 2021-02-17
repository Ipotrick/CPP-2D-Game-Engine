#pragma once

#include <random>

#include "../engine/EngineCore.hpp"
#include "../engine/entity/EntityComponentManager.hpp"
#include "../engine/collision/CoreComponents.hpp"
#include "../engine/collision/CollisionSystem.hpp"

#include "../engine/rendering/OpenGLAbstraction/OpenGLTexture.hpp"

#include "PheroGrid.hpp"

struct Ant {
	Vec2 size{ 2,1 };
	f32 speed{ 20 };
	f32 craziness{ 0.15 };
	f32 viewRange{ 7 };
	f32 footAmountTransporing{ 0.0f };
	LapTimer pheromoneLapTimer{ 0.1 };
	LapTimer changeDirTimer{ 0.1f };
	f32 confidence{ 0.0f };
	f32 timeSinceLastSource{ 0.0f };
	f32 pherpmonePenaltyLastUpdate{ 0.0f };
};

struct Food {
	f32 amount{ 20 };
};

struct Nest {
	f32 amount = 20;
};

struct Barrier {
	u32 dummy;
};

class AntsWorld {
public:

	void spawnAnt(Transform t, f32 viewRange)
	{
		const auto ent = ecm.create();
		ecm.addComp(ent, t);
		ecm.addComp(ent, Collider{ Vec2{viewRange,viewRange}, Form::Circle, true });
		ecm.addComp(ent, Movement{ Vec2{1,1} });
		ecm.addComp(ent, PhysicsBody{});
		Ant ant{ .viewRange = viewRange };
		ant.pheromoneLapTimer.getLaps(rand() % 1000 / 1000.0f);
		ant.changeDirTimer.getLaps(rand() % 1000 / 1000.0f);
		ecm.addComp(ent, ant);
		ecm.spawn(ent);
	}

	void spawnBarrier(Vec2 pos, Vec2 size)
	{
		EntityHandle ent = ecm.create();
		ecm.addComp(ent, Transform{ pos });
		ecm.addComp(ent, Collider{ size, Form::Circle });
		ecm.addComp(ent, PhysicsBody{});
		ecm.addComp(ent, Barrier{});
		ecm.spawn(ent);
	}

	void spawnFood(Vec2 pos, f32 amount = 10)
	{
		const f32 size = sqrt(amount);
		EntityHandle ant = ecm.create();
		ecm.addComp(ant, Transform{ pos });
		ecm.addComp(ant, Collider{ Vec2{size,size}, Form::Circle });
		ecm.addComp(ant, PhysicsBody{});
		ecm.addComp(ant, Food{.amount=amount});
		ecm.spawn(ant);
	}


	EntityComponentManager<
		ComponentStoragePagedIndexing<Transform>,
		ComponentStoragePagedIndexing<Collider>,
		ComponentStoragePagedIndexing<CollisionsToken>,
		ComponentStoragePagedIndexing<PhysicsBody>,
		ComponentStoragePagedIndexing<Movement>,
		ComponentStoragePagedSet<Ant>,
		ComponentStoragePagedSet<Food>,
		ComponentStoragePagedSet<Nest>,
		ComponentStoragePagedSet<Barrier>

	> ecm;


	void updateFood(f32 deltaTime)
	{
		for (auto ent : ecm.entityView<Food>()) {
			Food& f = ecm.getComp<Food>(ent);
			if (f.amount <= 0.0f) {
				ecm.destroy(ent);
			}
			Collider& coll = ecm.getComp<Collider>(ent);

			const f32 size = sqrt(f.amount);
			coll.size = { size ,size };
		}
	}

	void updateNests(f32 dt)
	{
		for (auto ent : ecm.entityView<Nest>()) {
			Nest& nest = ecm.getComp<Nest>(ent);
			Transform& trans = ecm.getComp<Transform>(ent);
		}
	}

	void updateAnts(f32 deltaTime)
	{
		for (auto ent : ecm.entityView<Ant, Transform>()) {
			Ant& ant = ecm.getComp<Ant>(ent);
			Transform& transform = ecm.getComp<Transform>(ent);

			ant.confidence = std::max(ant.confidence - deltaTime * 0.005f, 0.00f);
			ant.timeSinceLastSource += deltaTime;

			updateAntPoMovement(ant, transform, deltaTime);

			if (u32 pherocount = ant.pheromoneLapTimer.getLaps(deltaTime)) {
				const f32 strength = std::pow(ant.confidence, 1.3f) * (1.0f - ant.pherpmonePenaltyLastUpdate);
				const f32 srcDist = ant.timeSinceLastSource;
				if (ant.footAmountTransporing > 0.0f) {
					foodTransportPherogrid.plantAt(transform.position, strength, srcDist);
				}
				else {
					nestPheromone.plantAt(transform.position, strength, srcDist);
				}
			}

			if (ant.changeDirTimer.getLaps(deltaTime) > 0 || true) {
				updateAntAI(ent.index, ant, transform);
			}
		}
	}

	void updateAntPoMovement(Ant& ant, Transform& transform, f32 dt) {
		transform.position += dt * transform.rotaVec.toUnitX0() * ant.speed;
		if (abs(transform.position).x > WORLD_DIMENSIONS || abs(transform.position.y) > WORLD_DIMENSIONS) {
			transform.rotaVec = transform.rotaVec * RotaVec2(180.0f);
			transform.position = clamp(transform.position, Vec2{ -1,-1 } *WORLD_DIMENSIONS, Vec2{ 1,1 } *WORLD_DIMENSIONS);
		}
	}

	void updateAntAI(u32 id, Ant& antComp, Transform& transComp)
	{
		const bool bSearchOrCarry = !antComp.footAmountTransporing > 0.0f;

		std::array<PheroGrid::PheroCell, 25> neighborPhero;
		std::array<PheroGrid::PheroCell, 25> otherPhero;
		if (!bSearchOrCarry) {
			// WE TRANSPORT FOOD WE GO ALONG THE SEARCH PHERO
			neighborPhero = nestPheromone.getNeighbors5(transComp.position + transComp.rotaVec.toUnitX0() * GRID_CELL_SIZE * 2.0f);
			otherPhero = foodTransportPherogrid.getNeighbors5(transComp.position);
		}
		else {
			// WE SEEK FOOD WE GO ALONG THE FOOD PHERO
			neighborPhero = foodTransportPherogrid.getNeighbors5(transComp.position + transComp.rotaVec.toUnitX0() * GRID_CELL_SIZE * 2.0f);
			otherPhero = nestPheromone.getNeighbors5(transComp.position);
		}

		Vec2 drill = { 0.0f, 0.0f };
		f32 pheroConcentration{ 0.0f };

		// the ants smell the nests
		if (!bSearchOrCarry) {
			for (auto [id, nest, trans] : ecm.entityComponentView<Nest, Transform>()) {
				const f32 reldistToNest = length(transComp.position - trans.position) / WORLD_DIMENSIONS;
				const f32 weight = (1.0f - reldistToNest) * (1.0f - reldistToNest) * 0.002;
				pheroConcentration += weight;
				drill += weight * trans.position;
			}
		}

		// we go to the strong phero that has the best distance to the source
		f32 minSrcDist{ std::numeric_limits<float>::max() };
		f32 maxSrcDist{ 0.0f };
		f32 weightSum{ 0.0f };
		for (auto [amount, srcDist, position] : neighborPhero) {
			minSrcDist = std::min(minSrcDist, srcDist);
			maxSrcDist = std::max(maxSrcDist, srcDist);
		}
		for (auto [amount, srcDist, position] : neighborPhero) {
			f32 srcDistWeight = (srcDist - minSrcDist) / (maxSrcDist - minSrcDist);
			srcDistWeight = 1.0f - std::clamp(srcDistWeight, 0.0f, 1.0f);
			srcDistWeight *= srcDistWeight;

			const f32 dist = length(position - transComp.position);
			const f32 pheroStr = (amount / dist * 0.2f);
			pheroConcentration += pheroStr;

			const f32 weight = pheroStr * srcDistWeight;
			drill += position * weight;
			weightSum += weight;
		}
		drill /= weightSum;

		// we dont lay pheros when there is no phero under us but beside us, we dont want to widen paths!!
		const Vec2 lateralNormal = rotate<90>(transComp.rotaVec.toUnitX0());
		f32 otherPheroWeight = 0.0f;
		f32 maxPheroConcentration = 0.0f;
		const f32 MAX_LATERAL_DIST = GRID_CELL_SIZE * 2.5f;
		for (auto [amount, srcDist, position] : otherPhero) {
			const f32 lateralDistToDir = std::fabs(dot(position - transComp.position, lateralNormal));

			const f32 weight = (lateralDistToDir - GRID_CELL_SIZE*0.5f) / MAX_LATERAL_DIST;
			otherPheroWeight += amount * weight;
			maxPheroConcentration = std::max(maxPheroConcentration, amount * (MAX_LATERAL_DIST - lateralDistToDir)/ MAX_LATERAL_DIST);
		}
		antComp.pherpmonePenaltyLastUpdate = std::clamp(otherPheroWeight*0.1f, 0.0f, 0.99f);

		for (const CollisionInfo& coll : collsys.collisions_view(id)) {
			const u32 other = coll.indexB;
			const Transform& otherTrasnform = ecm.getComp<Transform>(other);
			const bool bTouching = (coll.clippingDist >= antComp.viewRange * 0.5 - 1);
			if (dot(transComp.rotaVec.toUnitX0(), otherTrasnform.position - transComp.position) > 0.0f) {
				if (Food* food = ecm.getIf<Food>(other); food) {
					if (bTouching) {
						if (antComp.footAmountTransporing == 0.0f) {
							food->amount -= 1;
							antComp.footAmountTransporing += 1;
						}
						transComp.rotaVec = RotaVec2::fromUnitX0(coll.normal[0]);
						antComp.confidence = 1.0f;
						antComp.timeSinceLastSource = 0.0f;
						return;
					}
					else if (bSearchOrCarry) {
						pheroConcentration = 1;
						drill = otherTrasnform.position;
					}
				}
				else if (Nest* nest = ecm.getIf<Nest>(other); nest) {
					if (bTouching) {
						nest->amount += antComp.footAmountTransporing;
						antComp.footAmountTransporing = 0.0f;
						transComp.rotaVec = RotaVec2::fromUnitX0(coll.normal[0]);
						antComp.confidence = 1.0f;
						antComp.timeSinceLastSource = 0.0f;
						return;
					}
					else if (!bSearchOrCarry) {
						pheroConcentration = 1;
						drill = otherTrasnform.position;
					}
				}
				else if (ecm.hasComp<Barrier>(other) && bTouching) {
					Vec2 dirVec = transComp.rotaVec.toUnitX0();
					const Vec2 normal = normalize(coll.normal[0] + coll.normal[1]);
					if (dot(dirVec, normal) < 0.0f) {

						transComp.rotaVec = RotaVec2::fromUnitX0(normal);
					}
				}
			}
		}

		if (pheroConcentration > 0.0f) {
			const f32 drillToPheroo = std::min(pheroConcentration, 1.0f);
			const Vec2 dirToPhero = normalize(drill - transComp.position);
			const Vec2 newDir = normalize(dirToPhero * drillToPheroo + transComp.rotaVec.toUnitX0());
			transComp.rotaVec = RotaVec2::fromUnitX0(newDir);
		}

		transComp.rotaVec = RotaVec2::fromUnitX0(normalize(transComp.rotaVec.toUnitX0() + RotaVec2{ f32(rand() % 360) }.toUnitX0() * antComp.craziness));
	}



	CollisionSystem collsys{ecm.submodule<COLLISION_SECM_COMPONENTS>()};
	static const u32 WORLD_DIMENSIONS{ 600 };
	DefaultRenderer renderer;
	const s32 GRID_CELL_SIZE{ 6 };
	PheroGrid foodTransportPherogrid{ WORLD_DIMENSIONS/ GRID_CELL_SIZE,WORLD_DIMENSIONS/ GRID_CELL_SIZE,GRID_CELL_SIZE };
	PheroGrid nestPheromone{ WORLD_DIMENSIONS/ GRID_CELL_SIZE,WORLD_DIMENSIONS/ GRID_CELL_SIZE, GRID_CELL_SIZE };
};