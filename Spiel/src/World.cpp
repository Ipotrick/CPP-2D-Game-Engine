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
	for (auto& el : entities) {
		res.push_back({ el.first, el.second.getCollidablePtr() });
	}
	return res;
}

void World::spawnEntity(Entity const& ent_, CompDataDrawable const& draw_)
{
	entities.insert( {nextID, ent_ });
	drawableCompCtrl.registerEntity( nextID, draw_);
	nextID++;
}

void World::despawn(uint32_t entitiy_id)
{
	auto iter = entities.find(entitiy_id);
	if (iter != entities.end()) {
		despawnList.push_back(entitiy_id);
	}
}

void World::executeDespawns()
{
	for (int entitiy_id : despawnList) {
		auto iter = entities.find(entitiy_id);
		if (iter != entities.end()) {
			entities.erase(iter);
		}
	}
	despawnList.clear();
}

void World::deregisterDespawnedEntities()
{
	for (int entitiy_id : despawnList) {
		auto iter = entities.find(entitiy_id);
		if (iter != entities.end()) {
			drawableCompCtrl.deregisterEntity(iter->first);
			playerCompCtrl.deregisterEntity(iter->first);
			healthCompCtrl.deregisterEntity(iter->first);
			ageCompCtrl.deregisterEntity(iter->first);
			bulletCompCtrl.deregisterEntity(iter->first);
		}
	}
}