#include "World.h"

std::vector<Drawable> World::getDrawableVec()
{
	std::vector<Drawable> res;
	res.reserve(entities.size());
	for (int id = 1; id < drawableCompCtrl.componentData.size(); id++) {
		if (drawableCompCtrl.isRegistered(id)) {
			res.push_back(buildDrawable(id, *getEntityPtr(id), drawableCompCtrl.getComponent(id)));
		}
	}
	return res;
}

Light World::buildLight(uint32_t id, Entity const& ent_, CompDataLight const& light_)
{
	return Light(ent_.position, ent_.size.x * 0.5f, id, light_.color);
}

std::vector<Light> World::getLightVec()
{
	std::vector<Light> res;
	for (auto iterA = lightCompCtrl.componentData.begin(); iterA != lightCompCtrl.componentData.end(); ++iterA) {
		res.push_back(buildLight(iterA->first, *getEntityPtr(iterA->first), iterA->second));
	}
	return res;
}

std::vector<std::tuple<uint32_t, Collidable*>> World::getCollidablePtrVec()
{
	std::vector<std::tuple<uint32_t, Collidable*>> res;
	res.reserve(entities.size());
	for (int id = 1; id < entities.size(); id++) {
		if (entities[id].first) {
			res.push_back({ id, entities[id].second.getCollidablePtr() });
		}
	}
	return res;
}

void World::spawnEntity(Entity const& ent, Draw const& draw) {
	if (!ent.isDynamic()) staticSpawnOrDespawn = true;
	if (emptySlots.size() > 0) {
		uint32_t id = emptySlots.front();
		emptySlots.pop();
		entities[id].first = true;
		entities[id].second = ent;
		lastID = id;
	}
	else {
		entities.push_back({ true,ent });
		lastID = entities.size() - 1;
	}

	drawableCompCtrl.registerEntity(lastID, draw);
}

void World::spawnSolidEntity(Entity ent, Draw const& draw, SolidBody solid)
{
	ent.solid = true;
	spawnEntity(ent, draw);

	if (solid.momentOfInertia == 0.0f) {
		solid.momentOfInertia = 1.0f / 12.0f * solid.mass * powf(std::max(ent.getSize().x, ent.getSize().y), 2) * 2;	//calculate moment of inertia I = 1/12 * m * l^2
	}
	solidBodyCompCtrl.registerEntity(lastID, solid);
}

void World::spawnSlave(Entity ent, Draw const& draw, uint32_t ownerID, vec2 relativePos, float relativeRota) {
	ent.ownerID = ownerID;
	spawnEntity(ent, draw);

	auto& ownerComposit = composit4CompCtrl.getComponent(ownerID);
	//find free slaveSlot
	int i = 0;
	while (i < 4) {
		if (ownerComposit.slaves[i].id == 0) {
			break;
		}
		else {
			i++;
		}
	}
	assert(i < 4);	//spawned more slaves than can be hold

	ownerComposit.slaves[i] = Composit<4>::Slave(getLastID(), relativePos, relativeRota);
}

void World::spawnSolidSlave(Entity ent, Draw const& draw, uint32_t ownerID, vec2 relativePos, float relativeRota) {
	ent.solid = true;
	spawnSlave(ent, draw, ownerID, relativePos, relativeRota);
}

void World::despawn(uint32_t entitiy_id) {
	if (entitiy_id < entities.size() && entities[entitiy_id].first) {
		despawnList.push_back(entitiy_id);
	}
}

void World::executeDespawns() {
	for (int entity_id : despawnList) {
		if (!entities[entity_id].second.isDynamic()) staticSpawnOrDespawn = true;
		entities[entity_id].first = false;
		emptySlots.push(entity_id);
	}
	despawnList.clear();
}

void World::slaveOwnerDespawn() {
	despawnList.reserve(entities.size());	//make sure the iterator stays valid 
	for (auto iter = despawnList.begin(); iter != despawnList.end(); ++iter) {
		assert(entities[*iter].first);
		//if the ent is an owner it despawns its slaves on destruction
		auto owner = composit4CompCtrl.getComponentPtr(*iter);
		if (owner != nullptr) {
			for (int i = 0; i < 4; ++i) {
				despawn(owner->slaves[i].id);
			}
		}
		//if ent is a slave it clears its refference of the owner on despawn
		if (getEntity(*iter).isSlave()) {
			auto owner = composit4CompCtrl.getComponentPtr(getEntity(*iter).getOwnerID());
			for (int i = 0; i < 4; ++i) {
				if (owner->slaves[i].id == *iter) {
					owner->slaves[i].id = 0;
					break;
				}
			}
		}
	}
}

void World::deregisterDespawnedEntities() {
	for (int entity_id : despawnList) {
		solidBodyCompCtrl.deregisterEntity(entity_id);
		drawableCompCtrl.deregisterEntity(entity_id); 
		composit4CompCtrl.deregisterEntity(entity_id);
		lightCompCtrl.deregisterEntity(entity_id);
		playerCompCtrl.deregisterEntity(entity_id);
		healthCompCtrl.deregisterEntity(entity_id);
		ageCompCtrl.deregisterEntity(entity_id);
		bulletCompCtrl.deregisterEntity(entity_id);
	}
}

size_t const World::getEntCount() {
	return entities.size() - emptySlots.size();
}

size_t const World::getEntMemSize() {
	return entities.size();
}