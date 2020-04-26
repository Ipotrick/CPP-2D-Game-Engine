#include "World.h"
#include "Physics.h"

ent_id_t World::createEnt() {
	if (!emptySlots.empty()) {
		ent_id_t id = emptySlots.front();
		emptySlots.pop();
		entities[id].setValid(true);
		entities[id].setDestroyMark(false);
		entities[id].setSpawned(false);
		lastID = id;
	}
	else {
		entities.emplace_back( true );
		lastID = static_cast<ent_id_t>(entities.size() - 1);
	}
	return lastID;
}

void World::enslaveEntTo(ent_id_t slave, ent_id_t owner, Vec2 relativePos, float relativeRota)
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

	ownerComposit.slaves[i] = Composit<4>::Slave(getLastEntID(), relativePos, relativeRota);
	if (!hasComp<Slave>(slave)) addComp<Slave>(slave);
	getComp<Slave>(slave) = Slave(owner);
}

void World::destroy(ent_id_t entitiy_id) {
	if (entitiy_id < entities.size() && !entities[entitiy_id].isDestroyMarked()) {
		assert(entities[entitiy_id].isValid());
		entities[entitiy_id].setDestroyMark(true);
		despawnList.push_back(entitiy_id);
	}
}



void World::executeDestroys() {
	for (ent_id_t entity : despawnList) {
		if (hasComp<Collider>(entity) && !hasComp<Movement>(entity)) { staticEntitiesChanged = true; }
		entities[entity].setValid(false);
		entities[entity].setDestroyMark(false);
		entities[entity].setSpawned(false);
		emptySlots.push(entity);
	}
	despawnList.clear();
}

void World::resetStaticsChangedFlag()
{
	staticEntitiesChanged = false;
}

void World::slaveOwnerDestroy() {
	despawnList.reserve(entities.size());	//make sure the iterator stays valid
	for (auto iter = despawnList.begin(); iter != despawnList.end(); ++iter) {
		assert(entities[*iter].isValid());
		//if the ent is an owner it despawns its slaves on destruction
		if (hasComp<Composit<4>>(*iter)) {
			auto& owner = getComp<Composit<4>>(*iter);
			for (int i = 0; i < 4; ++i) {
				destroy(owner.slaves[i].id);
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

void World::deregisterDestroyedEntities() {
	for (ent_id_t entity : despawnList) {
		for_each(componentStorageTuple, [&](auto& componentStorage) {
			componentStorage.remove(entity);
			});
	}
}

size_t const World::getEntCount() {
	return entities.size() - emptySlots.size();
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
		
		auto player = createEnt(); 
		addComp<Base>(player, Base({ 0,0 }, 0));
		addComp<Movement>(player, Movement(0.0f, 0.0f ));
		addComp<Draw>(player, Draw(Vec4(1, 1, 1, 1), scaleEnt, 0.6f, Form::RECTANGLE));
		addComp<Collider>(player, Collider(scaleEnt, Form::RECTANGLE));
		addComp<PhysicsBody>(player, PhysicsBody(0.1f, 60, calcMomentOfIntertia(60, scaleEnt),0.5f));
		addComp<Player>(player, Player());
		spawn(player);
		
		auto slave = createEnt();
		addComp<Base>(slave, Base({ 0,0 }, 0));
		addComp<Movement>(slave);
		addComp<PhysicsBody>(slave);
		addComp<Collider>(slave, Collider({scaleEnt.x}, Form::CIRCLE));
		addComp<Draw>(slave, Draw(Vec4(0, 0, 0, 1), {scaleEnt.x}, 0.49f, Form::CIRCLE));
		//addComp<TextureRef>(slave, TextureRef("test.png"));
		enslaveEntTo(slave, player, Vec2(0.0f, -0.4f), 45.0f);

		slave = createEnt();
		addComp<Base>(slave);
		addComp<Movement>(slave);
		addComp<PhysicsBody>(slave);
		addComp<Slave>(slave);
		addComp<Collider>(slave, Collider({ scaleEnt.x * 1 / sqrtf(2.0f) }, Form::RECTANGLE));
		addComp<Draw>(slave, Draw(Vec4(0,0,0,1), { scaleEnt.x * 1 / sqrtf(2.0f) }, 0.49f, Form::RECTANGLE));
		//addComp<TextureRef>(slave, TextureRef("test.png"));
		enslaveEntTo(slave, player, Vec2(0.0f, 0.4f), 45.0f);
		
		Vec2 scaleEnemy{ 5.4f, 1.4f };
		auto enemy = createEnt();
		addComp<Base>(enemy, Base({ 0,0 }, 0));
		addComp<Movement>(enemy, Movement(0.0f, 0.0f));
		addComp<Draw>(enemy, Draw(Vec4(1, 1, 1, 1), scaleEnemy, 0.4f, Form::RECTANGLE));
		addComp<Collider>(enemy, Collider(scaleEnemy, Form::RECTANGLE));
		addComp<PhysicsBody>(enemy, PhysicsBody(0.0f, 470, calcMomentOfIntertia(470, scaleEnemy),0.5f));
		addComp<Health>(enemy, Health(100));
		addComp<Enemy>(enemy, player);
		addComp<TextureRef>(enemy, TextureRef("test.png", Vec2(1.f / 16.f * 3.f, 1.f / 16.f * 15.f), Vec2(1.f / 16.f * 4.f, 1.f / 16.f * 16.f)));
		spawn(enemy);

		Collider	wallCollider(Vec2(0.4f, 10.0f), Form::RECTANGLE);
		PhysicsBody	wallSolidBody(0.5f, 1'000'000'000'000'000.0f, calcMomentOfIntertia(1'000'000'000'000'000.0f, Vec2(0.4f, 10.0f)), 1.0f);
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

		int num = 4;
		Vec2 scale = Vec2(0.3f, 0.3f);
		Collider trashCollider = Collider(scale, Form::RECTANGLE);
		Draw trashDraw = Draw(Vec4(0.0f, 0.0f, 0.0f, 1), scale, 0.5f, Form::RECTANGLE, true);
		PhysicsBody trashSolidBody(0.5f, 8.0f, calcMomentOfIntertia(8,scale) * 10, 0.5f);
		for (int i = 0; i < num; i++) {
			if (i % 2) {
				trashCollider.form = Form::CIRCLE;
				trashDraw.form = Form::CIRCLE;
			}
			else {
				trashCollider.form = Form::RECTANGLE;
				trashDraw.form = Form::RECTANGLE;
			}


			Vec2 position = { static_cast<float>(rand() % 1000 / 500.0f - 1.0f) * 4.6f, static_cast<float>(rand() % 1000 / 500.0f - 1.0f) * 4.6f };
			auto trash = createEnt();
			addComp<Base>(trash, Base(position, RotaVec2(0)));
			addComp<Movement>(trash, Movement(rand() % 1000 / 10000.0f - 0.05f, rand() % 1000 / 10000.0f - 0.05f));
			addComp<Collider>(trash, trashCollider);
			addComp<Draw>(trash, trashDraw);
			addComp<PhysicsBody>(trash, trashSolidBody);
			addComp<Health>(getLastEntID(), Health(100));
			spawn(trash);
		}
	}
}
