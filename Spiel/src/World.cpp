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
	if (emptySlots.size() > 0) {
		uint32_t id = emptySlots.front();
		emptySlots.pop();
		entities[id].first = true;
		entities[id].second = ent;
		lastID = id;
	}
	else {
		entities.push_back({ true,ent });
		lastID = nexBacktID;
		nexBacktID++;
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
		entities[entity_id].first = false;
		emptySlots.push(entity_id);
	}
	despawnList.clear();
}

void World::slaveOwnerDespawn() {
	despawnList.reserve(entities.size());	//make sure the iterator stays valid 
	for (auto iter = despawnList.begin(); iter != despawnList.end(); ++iter) {
		assert(entities[entity_id].first);
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

void World::loadMap(std:: string_view mapname_) {
	std::ifstream mapData("mapDateien.json");
	if (mapData.good()) {

	}
	else
	{

		vec2 scaleEnt = { 0.4f, 0.8f };
		auto cEnt = Entity(vec2(0, 0), 0.0f, Collidable(scaleEnt, Form::RECTANGLE, true, vec2(3, 0)));
		auto cDraw = Draw(vec4(0, 0, 0, 1), scaleEnt, 0.6f, Form::RECTANGLE, true);
		cEnt.rotation = 0.0;
		spawnSolidEntity(cEnt, cDraw, SolidBody(0.5f, 70.0f));
		addComp<Player>(getLastID(), Player());
		auto playerID = getLastID();
		addComp<Composit<4>>(playerID, Composit<4>());

		auto slaveC = Entity(vec2(0.5, 0), 0.0f, Collidable(vec2(scaleEnt), Form::CIRCLE, true, vec2(3, 0)));
		auto slaveD = Draw(vec4(0, 0, 0, 1), vec2(scaleEnt), 0.59f, Form::CIRCLE, true);
		spawnSolidSlave(slaveC, slaveD, playerID, vec2(0, -0.4), 90);
		spawnSolidSlave(slaveC, slaveD, playerID, vec2(0, 0.4), 90);

		vec2 scalePortal = { 28, 28 };
		Entity portalC = Entity(vec2(-4, -4), 0, Collidable(scalePortal, Form::CIRCLE, true));
		Draw portalD = Draw(vec4(1, 0, 0, 0.5f), vec2(3, 3), 0.3f, Form::CIRCLE);
		spawnEntity(portalC, portalD);

		portalC.size = vec2(3, 3);
		portalC.position = vec2(4, 4);
		portalD.color = vec4(0, 0, 1, 0.5f);
		portalD.drawingPrio = 0.31f;
		spawnEntity(portalC, portalD);

		Entity wallC = Entity(vec2(0, 0), 0, Collidable(vec2(0.4f, 10), Form::RECTANGLE, false, vec2(0, 0)));
		Draw wallD = Draw(vec4(0, 0, 0, 1), vec2(0.4f, 10), 0.5f, Form::RECTANGLE, true);
		for (int i = 0; i < 4; i++) {
			float rotation = 90.0f * i;
			wallC.position = rotate(vec2(-5.f, 0), rotation);
			wallC.rotation = rotation;
			spawnSolidEntity(wallC, wallD, SolidBody(0.3f, 1'000'000'000'000'000.0f));
		}

		int num = 3000;

		vec2 scale = vec2(0.08f, 0.08f);
		Entity trashEntC = Entity(vec2(0, 0), 0.0f, Collidable(scale, Form::CIRCLE, true, vec2(0, 0)));
		Draw trashEntD = Draw(vec4(1, 102.0f / 255.0f, 0, 1), scale, 0.5f, Form::CIRCLE, true);
		auto slaveEntC = trashEntC;
		trashEntC.form = Form::RECTANGLE;
		auto slaveEntD = trashEntD;
		slaveEntD.color = vec4(0, 0, 0, 1);
		slaveEntD.drawingPrio += 0.01f;
		slaveEntD.form = Form::RECTANGLE;
		auto trashSolid = SolidBody(0.1f, 0.5f);
		trashSolid.momentOfInertia = 0.11f;
		for (int i = 0; i < num; i++) {

			trashEntC.position = { static_cast<float>(rand() % 1000 / 500.0f - 1.0f) * 4.6f, static_cast<float>(rand() % 1000 / 500.0f - 1.0f) * 4.6f };

			spawnSolidEntity(trashEntC, trashEntD, trashSolid);
			addComp<Health>(getLastID(), Health(100));
			spawnSolidSlave(slaveEntC, slaveEntD, getLastID(), vec2(0.04f, 0), 0);
		}
	}
}