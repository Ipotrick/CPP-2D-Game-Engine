#include "World.h"
#include "Physics.h"

entity_handle World::createEnt() {
	if (!freeHandleQueue.empty()) {
		entity_handle id = freeHandleQueue.front();
		freeHandleQueue.pop();
		entities[id].setValid(true);
		entities[id].setDestroyMark(false);
		entities[id].setSpawned(false);
		latestHandle = id;
	}
	else {
		entities.emplace_back( true );
		latestHandle = static_cast<entity_handle>(entities.size() - 1);
	}
	return latestHandle;
}

void World::enslaveEntTo(entity_handle slave, entity_handle owner, Vec2 relativePos, float relativeRota)
{
	if (!hasComp<Composit<4>>(owner)) {
		addComp<Composit<4>>(owner, Composit<4>());
	}
	auto& ownerComposit = getComp<Composit<4>>(owner);
	//find free slaveSlot
	int i = 0;
	while (i < 4) {
		if (ownerComposit.slaves[i].handle == 0) {
			break;
		}
		else {
			i++;
		}
	}
	assert(i < 4);	//spawned more slaves than can be hold

	ownerComposit.slaves[i] = Composit<4>::Slave(getLastEntID(), relativePos, relativeRota);
	if (!hasComp<Slave>(slave)) addComp<Slave>(slave);
	getComp<Slave>(slave) = Slave(owner);
}

void World::destroy(entity_handle entitiy_id) {
	if (entitiy_id < entities.size() && !entities[entitiy_id].isDestroyMarked()) {
		assert(entities[entitiy_id].isValid());
		entities[entitiy_id].setDestroyMark(true);
		despawnList.push_back(entitiy_id);
	}
}

void World::spawnLater(entity_handle entity)
{
	spawnLaterList.emplace_back(entity);
}

void World::respawnLater(entity_handle entity)
{
	spawnLaterList.emplace_back(entity);
}

entity_id World::identify(entity_handle entity)
{
	if (!doesEntExist(entity)) return { 0 };
	if (handleToId.size() != entities.size()) handleToId.resize(entities.size(), { 0 });
	if (handleToId[entity].id != 0) /* does the handle allready have an id? */ {
		return handleToId[entity];
	}
	else {
		// generate id for entity
		if (!freeIdQueue.empty()) {
			auto idToken = freeIdQueue.front();
			freeIdQueue.pop();
			idToHandle[idToken.id] = entity;
			handleToId[entity] = idToken;
			return idToken;
		}
		else {
			idToHandle.push_back(entity);
			entity_id idToken = entity_id(idToHandle.size() - 1);
			handleToId[entity] = { idToHandle[idToken.id] };
			return idToken;
		}
	}
}

void World::executeDestroys() {
	for (entity_handle entity : despawnList) {
		if (hasComp<Collider>(entity) && !hasComp<Movement>(entity)) { staticEntitiesChanged = true; }
		// reset id references:
		if (hasID(entity)) {
			auto idToken = handleToId[entity];
			freeIdQueue.push(idToken);
			handleToId[entity] = { 0 };
			idToHandle[idToken.id] = 0;
		}
		// reset status of handle:
		entities[entity].setValid(false);
		entities[entity].setDestroyMark(false);
		entities[entity].setSpawned(false);
		freeHandleQueue.push(entity);
	}
	despawnList.clear();
}

void World::sortFreeHandleQueue()
{
	std::vector<entity_handle> queueVec;
	queueVec.reserve(freeHandleQueue.size());
	while (!freeHandleQueue.empty()) {
		queueVec.push_back(freeHandleQueue.front());
		freeHandleQueue.pop();
	}
	std::stable_sort(queueVec.begin(), queueVec.end(), [&](entity_handle const& a, entity_handle const& b) { return a < b; });
	for (auto slot : queueVec) freeHandleQueue.push(slot);
}

void World::sortFreeIDQueue()
{
	std::vector<entity_id> queueVec;
	queueVec.reserve(freeIdQueue.size());
	while (!freeIdQueue.empty()) {
		queueVec.push_back(freeIdQueue.front());
		freeIdQueue.pop();
	}
	std::stable_sort(queueVec.begin(), queueVec.end(), [&](entity_id const& a, entity_id const& b) { return a.id < b.id; });
	for (auto id : queueVec) freeIdQueue.push(id);
}

void World::resetStaticsChangedFlag()
{
	staticEntitiesChanged = false;
}

void World::tick()
{
	executeDelayedSpawns();
	slaveOwnerDestroy();
	deregisterDestroyedEntities();
	executeDestroys();
}

void World::slaveOwnerDestroy() {
	despawnList.reserve(entities.size());	//make sure the iterator stays valid
	for (auto iter = despawnList.begin(); iter != despawnList.end(); ++iter) {
		assert(entities[*iter].isValid());
		//if the ent is an owner it despawns its slaves on destruction
		if (hasComp<Composit<4>>(*iter)) {
			auto& owner = getComp<Composit<4>>(*iter);
			for (int i = 0; i < 4; ++i) {
				destroy(owner.slaves[i].handle);
			}
		}
		//if ent is a slave it clears its refference of the owner on despawn
		
		if (hasComp<Slave>(*iter)) {
			auto slaveComp = getComp<Slave>(*iter);
			auto owner = getComp<Composit<4>>(slaveComp.ownerHandle);
			for (int i = 0; i < 4; ++i) {
				if (owner.slaves[i].handle == *iter) {
					owner.slaves[i].handle = 0;
					break;
				}
			}
		}
	}
}

void World::deregisterDestroyedEntities() {
	for (entity_handle entity : despawnList) {
		for_each(componentStorageTuple, [&](auto& componentStorage) {
			componentStorage.remove(entity);
			});
	}
}

void World::executeDelayedSpawns()
{
	for (auto ent : spawnLaterList) {
		spawn(ent);
	}
	spawnLaterList.clear();
}

size_t const World::getEntCount() {
	return entities.size() - freeHandleQueue.size();
}

size_t const World::getEntMemSize() {
	return entities.size();
}

void World::staticsChanged()
{
	staticEntitiesChanged = true;
}

bool World::didStaticsChange()
{
	return staticEntitiesChanged;
}

float randomFloatd(float MaxAbsVal) {
	float randomNum = rand() % 1'000 / 1'000.0f;
	randomNum -= 0.5f;
	randomNum *= MaxAbsVal;
	return randomNum;
}

void World::loadMap(std:: string mapname_) {
	std::ifstream mapData(mapname_);
	if (mapData.good()) {

	}
	else
	{
		Vec2 scaleEnt = { 0.4f, 0.8f };
		uniformsPhysics.friction = 0.06f;
		uniformsPhysics.linearEffectDir = Vec2(0, -1);
		uniformsPhysics.linearEffectAccel = 1.f;
		
		auto player = createEnt(); 
		addComp<Base>(player, Base({ 0,0 }, 0));
		addComp<Movement>(player, Movement(0.0f, 0.0f ));
		addComp<Draw>(player, Draw(Vec4(1, 1, 1, 1), scaleEnt, 0.6f, Form::RECTANGLE));
		addComp<Collider>(player, Collider(scaleEnt, Form::RECTANGLE));
		addComp<PhysicsBody>(player, PhysicsBody(0.1f, 60, calcMomentOfIntertia(60, scaleEnt),100));
		addComp<TextureRef>(player, TextureRef("Dir.png"));
		addComp<Player>(player, Player());
		spawn(player);

		auto slave = createEnt();
		addComp<Base>(slave);
		addComp<Movement>(slave);
		addComp<PhysicsBody>(slave);
		addComp<Slave>(slave);
		addComp<Collider>(slave, Collider({ scaleEnt.x * 1 / sqrtf(2.0f) }, Form::RECTANGLE));
		addComp<Draw>(slave, Draw(Vec4(0,0,0,1), { scaleEnt.x * 1 / sqrtf(2.0f) }, 0.6f, Form::RECTANGLE));
		enslaveEntTo(slave, player, Vec2(0.0f, 0.4f), 45.0f);
		spawn(slave);

		Vec2 scaleLegs{ 0.1, 0.2 };
		slave = createEnt();
		addComp<Base>(slave);
		addComp<Movement>(slave);
		addComp<PhysicsBody>(slave);
		addComp<Slave>(slave);
		addComp<Collider>(slave, Collider(scaleLegs, Form::RECTANGLE));
		addComp<Draw>(slave, Draw(Vec4(0, 0, 0, 1), scaleLegs, 0.6f, Form::RECTANGLE));
		enslaveEntTo(slave, player, Vec2(0.2f, -0.4f), 30.0f);
		spawn(slave);


		slave = createEnt();
		addComp<Base>(slave);
		addComp<Movement>(slave);
		addComp<PhysicsBody>(slave);
		addComp<Slave>(slave);
		addComp<Collider>(slave, Collider(scaleLegs, Form::RECTANGLE));
		addComp<Draw>(slave, Draw(Vec4(0, 0, 0, 1), scaleLegs, 0.6f, Form::RECTANGLE));
		enslaveEntTo(slave, player, Vec2(-0.2f, -0.4f), -30.0f);
		spawn(slave);
		
		/*Vec2 scaleEnemy{ 5.4f, 1.4f };
		auto enemy = createEnt();
		addComp<Base>(enemy, Base({ 0,0 }, 0));
		addComp<Movement>(enemy, Movement(0.0f, 0.0f));
		addComp<Draw>(enemy, Draw(Vec4(1, 1, 1, 1), scaleEnemy, 0.4f, Form::RECTANGLE));
		addComp<Collider>(enemy, Collider(scaleEnemy, Form::RECTANGLE));
		addComp<PhysicsBody>(enemy, PhysicsBody(0.0f, 470, calcMomentOfIntertia(470, scaleEnemy),10.f));
		addComp<Health>(enemy, Health(100));
		addComp<Enemy>(enemy, player);
		addComp<TextureRef>(enemy, TextureRef("test.png", Vec2(1.f / 16.f * 3.f, 1.f / 16.f * 15.f), Vec2(1.f / 16.f * 4.f, 1.f / 16.f * 16.f)));
		spawn(enemy);

		auto pinguin = createEnt();
		addComp<Base>(pinguin, Base(Vec2(3, 4), 0));
		addComp<Draw>(pinguin, Draw(Vec4(1, 1, 1, 0.5), Vec2(2, 3), 0.5F, Form::RECTANGLE));
		addComp<Collider>(pinguin, Collider(Vec2(2,3), Form::RECTANGLE));
		addComp<TextureRef>(pinguin, TextureRef("pingu.png"));
		addComp<PhysicsBody>(pinguin, PhysicsBody(1, 8, calcMomentOfIntertia(8, Vec2(2, 3)), 0.5));
		addComp<PhysicsBody>(pinguin);
		spawn(pinguin);*/

		Collider	wallCollider(Vec2(0.4f, 10.0f), Form::RECTANGLE);
		PhysicsBody	wallSolidBody(0.5f, 1'000'000'000'000'000.0f, calcMomentOfIntertia(1'000'000'000'000'000.0f, Vec2(0.4f, 10.0f)), 100.0f);
		Draw		wallDraw = Draw(Vec4(0, 0, 0, 1), Vec2(0.4f, 10.0f), 0.5f, Form::RECTANGLE, true);
		for (int i = 0; i < 4; i++) {
			auto wall = createEnt();
			std::cout << "wallid: " << wall << std::endl;
			float rotation = 90.0f * i;
			addComp<Base>(wall, Base(rotate(Vec2(-5.f, 0.0f), rotation), rotation));
			addComp<Collider>(wall, wallCollider); 
			addComp<PhysicsBody>(wall, wallSolidBody);
			addComp<Draw>(wall, wallDraw);
			//addComp<TextureRef>(wall, TextureRef("test.png",vec2(0,0), vec2(1,25)));
			spawn(wall);
		}

		int num = 0;
		Vec2 scale = Vec2(0.1f, 0.1f);
		Collider trashCollider = Collider(scale, Form::RECTANGLE);
		Draw trashDraw = Draw(Vec4(1.0f, 1.0f, 1.0f, 1), scale, 0.5f, Form::RECTANGLE, true);
		PhysicsBody trashSolidBody(0.9f, 1.0f, calcMomentOfIntertia(1,scale), 10.0f);
		for (int i = 0; i < num; i++) {
			if (i % 2) {
				trashCollider.form = Form::CIRCLE;
				trashDraw.form = Form::CIRCLE;
			}
			else {
				//trashCollider.form = Form::RECTANGLE;
				//trashDraw.form = Form::RECTANGLE;
			}


			Vec2 position = { static_cast<float>(rand() % 1000 / 500.0f - 1.0f) * 4.6f, static_cast<float>(rand() % 1000 / 500.0f - 1.0f) * 4.6f };
			auto trash = createEnt();
			addComp<Base>(trash, Base(position, RotaVec2(0)));
			addComp<Movement>(trash, Movement(rand() % 1000 / 10000.0f - 0.05f, rand() % 1000 / 10000.0f - 0.05f));
			addComp<Collider>(trash, trashCollider);
			addComp<Draw>(trash, trashDraw);
			addComp<PhysicsBody>(trash, trashSolidBody);
			addComp<Health>(trash, Health(100));
			addComp<TextureRef>(trash, TextureRef("Dir.png"));
			spawn(trash);
		}

		int num2 = 0;
		Vec2 scale2 = Vec2(0.04f, 0.04f);
		Collider trashCollider2 = Collider(scale2, Form::RECTANGLE);
		Draw trashDraw2 = Draw(Vec4(1.0f, 1.0f, 1.0f, 1), scale2, 0.5f, Form::RECTANGLE, true);
		PhysicsBody trashSolidBody2(0.9f, 1'000'000'000'000'000.0f, calcMomentOfIntertia(1, scale), 10.0f);
		for (int i = 0; i < num2; i++) {
			if (i % 2) {
				trashCollider2.form = Form::CIRCLE;
				trashDraw2.form = Form::CIRCLE;
			}
			else {
				trashCollider2.form = Form::RECTANGLE;
				trashDraw2.form = Form::RECTANGLE;
			}

			Vec2 position2 = { static_cast<float>(rand() % 1000 / 10.0f - 50.0f) * 4.6f, static_cast<float>(rand() % 1000 / 10.0f - 50.0f) * 4.6f };
			auto trash2 = createEnt();
			addComp<Base>(trash2, Base(position2, RotaVec2(0)));
			addComp<Collider>(trash2, trashCollider2);
			addComp<PhysicsBody>(trash2, trashSolidBody2);
			spawn(trash2);
		}
	}
}
