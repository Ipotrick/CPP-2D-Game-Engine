#include "World.h"

std::vector<Drawable> World::getDrawableVec()
{
	std::vector<Drawable> res;
	res.reserve(entities.size());
	for (auto iterA = drawableCompCtrl.componentData.begin(); iterA != drawableCompCtrl.componentData.end(); ++iterA) {
		res.push_back(buildDrawable(iterA->first, *getEntityPtr(iterA->first), iterA->second));
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

void World::spawnEntity(Entity const& ent_, CompDataDrawable const& draw_) {
	if (emptySlots.size() > 0) {
		uint32_t id = emptySlots.front();
		emptySlots.pop();
		entities[id].first = true;
		entities[id].second = ent_;
		lastID = id;
	}
	else {
		entities.push_back({ true,ent_ });
		lastID = nexBacktID;
		nexBacktID++;
	}

	drawableCompCtrl.registerEntity(lastID, draw_);
}

void World::spawnSolidEntity(Entity const& ent_, CompDataSolidBody solid, CompDataDrawable const& draw_)
{
	if (emptySlots.size() > 0) {
		uint32_t id = emptySlots.front();
		emptySlots.pop();
		entities[id].first = true;
		entities[id].second = ent_;
		lastID = id;
	}
	else {
		entities.push_back({ true,ent_ });
		lastID = nexBacktID;
		nexBacktID++;
	}

	drawableCompCtrl.registerEntity(lastID, draw_);
	solidBodyCompCtrl.registerEntity(lastID, solid);
}

void World::despawn(uint32_t entitiy_id)
{
	if (entitiy_id < entities.size() && entities[entitiy_id].first) {
		despawnList.push_back(entitiy_id);
	}
}

void World::executeDespawns()
{
	for (int entity_id : despawnList) {
		entities[entity_id].first = false;
		emptySlots.push(entity_id);
	}
	despawnList.clear();
}

void World::deregisterDespawnedEntities()
{
	for (int entity_id : despawnList) {
		if(entities[entity_id].first) {
			solidBodyCompCtrl.deregisterEntity(entity_id);
			drawableCompCtrl.deregisterEntity(entity_id);
			playerCompCtrl.deregisterEntity(entity_id);
			healthCompCtrl.deregisterEntity(entity_id);
			ageCompCtrl.deregisterEntity(entity_id);
			bulletCompCtrl.deregisterEntity(entity_id);
		}
	}
}