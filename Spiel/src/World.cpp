#include "World.h"
#include "Physics.h"

std::vector<Drawable> World::getDrawableVec()
{
	std::vector<Drawable> res;
	res.reserve(entities.size());
	for (auto iter = view<Draw>().begin(); iter != view<Draw>().end(); ++iter) {
		res.push_back(buildDrawable(iter.id(), *iter));
	}
	return res;
}

ent_id_t World::createEnt() {
	if (!emptySlots.empty()) {
		ent_id_t id = emptySlots.front();
		emptySlots.pop();
		entities[id].valid = true;
		lastID = id;
	}
	else {
		entities.push_back({ true });
		lastID = entities.size() - 1;
	}
	return lastID;
}

void World::enslaveEntTo(ent_id_t slave, ent_id_t owner, vec2 relativePos, float relativeRota)
{
	if (!hasComp<Composit<4>>(owner)) {
		addComp<Composit<4>>(owner, Composit<4>());
	}
	auto& ownerComposit = getComp<Composit<4>>(owner);
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
	if (!hasComp<Slave>(slave)) addComp<Slave>(slave);
	getComp<Slave>(slave) = Slave(owner);
}

void World::despawn(ent_id_t entitiy_id) {
	if (entitiy_id < entities.size() && entities[entitiy_id].valid) {
		despawnList.push_back(entitiy_id);
	}
}

void World::executeDespawns() {
	std::cout << "empty slots before: " << emptySlots.size() << std::endl;
	for (ent_id_t entity : despawnList) {
		if (hasComp<Collider>(entity) && !hasComp<Movement>(entity)) { staticSpawnOrDespawn = true; }
		entities[entity].valid = false;
		emptySlots.push(entity);
	}
	std::cout << "empty slots: " << emptySlots.size() << std::endl << std::endl;
	despawnList.clear();
}

void World::slaveOwnerDespawn() {
	despawnList.reserve(entities.size());	//make sure the iterator stays valid 
	for (auto iter = despawnList.begin(); iter != despawnList.end(); ++iter) {
		assert(entities[*iter].valid);
		//if the ent is an owner it despawns its slaves on destruction
		if (hasComp<Composit<4>>(*iter)) {
			auto& owner = getComp<Composit<4>>(*iter);
			for (int i = 0; i < 4; ++i) {
				despawn(owner.slaves[i].id);
			}
		}
		//if ent is a slave it clears its refference of the owner on despawn
		
		if (hasComp<Slave>(*iter)) {
			auto slaveComp = getComp<Slave>(*iter);
			auto owner = getComp<Composit<4>>(slaveComp.ownerID);
			for (int i = 0; i < 4; ++i) {
				if (owner.slaves[i].id == *iter) {
					owner.slaves[i].id = 0;
					break;
				}
			}
		}
	}
}

void World::deregisterDespawnedEntities() {
	for (ent_id_t entity : despawnList) {
		compStorage0.deregistrate(entity);
		compStorage1.deregistrate(entity);
		compStorage2.deregistrate(entity);
		compStorage3.deregistrate(entity);
		compStorage4.deregistrate(entity);
		compStorage5.deregistrate(entity);
		compStorage6.deregistrate(entity);
		compStorage7.deregistrate(entity);
		compStorage8.deregistrate(entity);
		compStorage9.deregistrate(entity);
		compStorage10.deregistrate(entity);
		compStorage11.deregistrate(entity);
	}
}

size_t const World::getEntCount() {
	return entities.size() - emptySlots.size();
}

size_t const World::getEntMemSize() {
	return entities.size();
}

void World::loadMap(std:: string mapname_) {
	std::ifstream mapData(mapname_);
	if (mapData.good()) {

	}
	else
	{
		vec2 scaleEnt = { 0.4f, 0.8f };
		
		auto player = createEnt();
		addComp<Base>(player, Base({ 0,0 }, 0));
		addComp<Movement>(player, Movement(0.0f,0.0f));
		addComp<Draw>(player, Draw(vec4(0, 0, 0, 1), scaleEnt, 0.6f, Form::RECTANGLE));
		addComp<Collider>(player, Collider(scaleEnt, Form::RECTANGLE, true));
		addComp<SolidBody>(player, SolidBody(0.5f, 70, calcMomentOfIntertia(70, scaleEnt)));
		addComp<Player>(player, Player());
		addComp<Composit<4>>(player, Composit<4>());

		vec2		slaveSlave = vec2(scaleEnt.x * 1 / sqrtf(2.0f));
		Collider	slaveCollider(slaveSlave, Form::RECTANGLE, true);
		Draw		slaveDraw(vec4(0, 0, 0, 1), slaveSlave, 0.49f, Form::RECTANGLE);

		auto slave = createEnt();
		addComp<Base>(slave);
		addComp<Movement>(slave);
		addComp<SolidBody>(slave);
		addComp<Collider>(slave, slaveCollider);
		addComp<Draw>(slave, slaveDraw);
		enslaveEntTo(slave, player, vec2(0, -0.4f), 45.0f);

		slave = createEnt();
		addComp<Base>(slave);
		addComp<Movement>(slave);
		addComp<SolidBody>(slave);
		addComp<Slave>(slave);
		addComp<Collider>(slave, slaveCollider);
		addComp<Draw>(slave, slaveDraw);
		enslaveEntTo(slave, player, vec2(0, 0.4f), 45.0f);

		Collider	wallCollider(vec2(0.4f, 10.0f), Form::RECTANGLE, false);
		SolidBody	wallSolidBody(0.3f, 1'000'000'000'000'000.0f, calcMomentOfIntertia(1'000'000'000'000'000.0f, vec2(0.4f, 10)));
		Draw		wallDraw = Draw(vec4(0, 0, 0, 1), vec2(0.4f, 10), 0.5f, Form::RECTANGLE, true);
		for (int i = 0; i < 4; i++) {
			auto wall = createEnt();
			addComp<Collider>(wall, wallCollider);
			addComp<SolidBody>(wall, wallSolidBody);
			addComp<Draw>(wall, wallDraw);
			float rotation = 90.0f * i;
			addComp<Base>(wall, Base(rotate(vec2(-5.f, 0), rotation), rotation));
		}

		int num = 10000;
		vec2 scale = vec2(0.05f, 0.05f);
		Collider trashCollider = Collider(scale, Form::CIRCLE, true);
		Draw trashDraw = Draw(vec4(0.0f, 0.0f, 0.0f, 1), scale, 0.5f, Form::CIRCLE, true);
		SolidBody trashSolidBody(0.5f, 1.0f, 0.1f);
		for (int i = 0; i < num; i++) {

			vec2 position = { static_cast<float>(rand() % 1000 / 500.0f - 1.0f) * 4.6f, static_cast<float>(rand() % 1000 / 500.0f - 1.0f) * 4.6f };
			auto trash = createEnt();
			addComp<Base>(trash, Base(position, 0.0f));
			addComp<Movement>(trash);
			addComp<Collider>(trash, trashCollider);
			addComp<Draw>(trash, trashDraw);
			addComp<SolidBody>(trash, trashSolidBody);
			addComp<Health>(getLastID(), Health(100));
		}
	}
}