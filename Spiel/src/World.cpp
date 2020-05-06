#include "World.h"
#include "Physics.h"

entity_handle World::createEnt() {
	if (!freeHandleQueue.empty()) {
		entity_handle handle = freeHandleQueue.front();
		freeHandleQueue.pop();
		entities[handle].setValid(true);
		entities[handle].setDestroyMark(false);
		entities[handle].setSpawned(false);
		latestHandle = handle;
	}
	else {
		entities.emplace_back( true );
		latestHandle = static_cast<entity_handle>(entities.size() - 1);
	}
	return latestHandle;
}

void World::linkBase(entity_handle slave, entity_handle owner, Vec2 relativePos, float relativeRota)
{
	if (hasntComp<Parent>(owner)) addComp<Parent>(owner);
	auto& parent = getComp<Parent>(owner);

	parent.children.push_back(identify(slave));

	if (hasntComp<BaseChild>(slave)) addComp<BaseChild>(slave);
	auto& baseChild = getComp<BaseChild>(slave);

	baseChild.relativePos = relativePos;
	baseChild.relativeRota = relativeRota;
	baseChild.parent = identify(owner);
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
	assert(doesEntExist(entity));
	if (handleToId.size() != entities.size()) handleToId.resize(entities.size(), entity_id(0,0) );
	if (handleToId[entity].id != 0) /* does the handle allready have an id? */ {
		return handleToId[entity];
	}
	else {
		// generate id for entity
		if (!freeIdQueue.empty()) {
			// reuse existing index of id vector
			auto idToken = freeIdQueue.front();
			freeIdQueue.pop();
			idToHandle[idToken.id] = entity;
			idVersion[idToken.id] += 1;	// for every reuse the version gets an increase
			handleToId[entity] = idToken;
			idToken.version = idVersion[idToken.id];
			return idToken;
		}
		else {
			// expand id vector
			idToHandle.push_back(entity);
			idVersion.emplace_back(0);
			entity_id idToken = entity_id(idToHandle.size() - 1, 0);
			handleToId[entity] = idToken;
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
			handleToId[entity] = entity_id(0, 0);
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
	despawnList.reserve(entities.size());	// make sure the iterators stay valid
	executeDelayedSpawns();
	childParentDestroy();
	parentChildDestroy();
	deregisterDestroyedEntities();
	executeDestroys();
	sortFreeHandleQueue();
	defragmentEntities();
}

void World::moveEntity(entity_handle start, entity_handle goal)
{
	assert(entities.size() > goal && entities[goal].isValid() == false);
	assert(entities.size() > start && entities[start].isValid() == true); 
	if (hasID(start)) {
		idToHandle[handleToId[start].id] = goal;
		handleToId[goal] = handleToId[start];
		handleToId[start].id = 0;
		
	}
	for_each(componentStorageTuple, [&](auto& componentStorage) {
		if (componentStorage.contains(start)) {
			componentStorage.insert(goal, componentStorage.get(start));
			componentStorage.remove(start);
			assert(componentStorage.contains(goal));
			assert(!componentStorage.contains(start));
		}
		});
	entities[goal].setValid(true);
	entities[goal].setSpawned(entities[start].isSpawned());
	entities[start].setValid(false);
	entities[start].setDestroyMark(false);
	entities[start].setSpawned(false);
}

entity_handle World::findBiggestValidHandle()
{
	for (int i = entities.size() - 1; i > 0; i--) {
		if (entities[i].isValid()) return i;
	}
	return 0;
}

void World::shrink() {
	auto lastEl = findBiggestValidHandle();
	entities.resize(lastEl + 1, EntityStatus(false));
	std::vector<entity_handle> handleVec;
	while (!freeHandleQueue.empty()) {
		handleVec.push_back(freeHandleQueue.front());
		freeHandleQueue.pop();
	}
	for (auto& el : handleVec) {
		if (el < entities.size()) freeHandleQueue.push(el);
	}
}

void World::defragmentEntities()
{
	int maxDefragEntCount;
	float minFragmentation;
	switch (defragMode) {
	case DefragMode::FAST:
		maxDefragEntCount = 10;
		minFragmentation = 0.1;
		break;
	case DefragMode::NONE:
		maxDefragEntCount = 0;
		minFragmentation = 1.01f;
		break;
	case DefragMode::LAZY:
		maxDefragEntCount = 50;
		minFragmentation = 0.5f;
		break;
	case DefragMode::MODERATE:
		maxDefragEntCount = 50;
		minFragmentation = 0.33333f;
		break;
	case DefragMode::EAGER:
		maxDefragEntCount = 50;
		minFragmentation = 0.3f;
		break;
	case DefragMode::AGRESSIVE:
		maxDefragEntCount = 50;
		minFragmentation = 0.1f;
		break;
	case DefragMode::COMPLETE:
		maxDefragEntCount = getEntMemSize();
		minFragmentation = 0.00001f;
		break;
	}

	if (getFragmentation() > minFragmentation) {
		shrink();
		
		for (int defragCount = 0; defragCount < maxDefragEntCount; defragCount++) {
			if (freeHandleQueue.empty()) break;
			auto biggesthandle = findBiggestValidHandle();
			moveEntity(biggesthandle, freeHandleQueue.front());
			freeHandleQueue.pop();
			shrink();
		}
	}
}

void World::childParentDestroy()
{
	for (auto& entity : despawnList) {
		if (hasComp<BaseChild>(entity)) {
			despawnList.push_back(getEnt(getComp<BaseChild>(entity).parent));
		}
	}
}

void World::parentChildDestroy() {
	for (auto& entity : despawnList) {
		if (hasComp<Parent>(entity)) {
			auto& parent = getComp<Parent>(entity);
			for (auto& child : parent.children) {
				despawnList.push_back(getEnt(child));
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
	return entities.size() - (freeHandleQueue.size() + 1);
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

float World::getFragmentation()
{
	return (float)freeHandleQueue.size() / (float)getEntMemSize();
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
		//uniformsPhysics.linearEffectDir = Vec2(0, -1);
		//uniformsPhysics.linearEffectAccel = 1.f;
		
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
		addComp<Collider>(slave, Collider({ scaleEnt.x * 1 / sqrtf(2.0f) }, Form::RECTANGLE));
		addComp<Draw>(slave, Draw(Vec4(0,0,0,1), { scaleEnt.x * 1 / sqrtf(2.0f) }, 0.6f, Form::RECTANGLE));
		linkBase(slave, player, Vec2(0.0f, 0.4f), 45.0f);
		spawn(slave);

		Vec2 scaleLegs{ 0.1, 0.2 };
		slave = createEnt();
		addComp<Base>(slave);
		addComp<Movement>(slave);
		addComp<PhysicsBody>(slave);
		addComp<Collider>(slave, Collider(scaleLegs, Form::RECTANGLE));
		addComp<Draw>(slave, Draw(Vec4(0, 0, 0, 1), scaleLegs, 0.6f, Form::RECTANGLE));
		linkBase(slave, player, Vec2(0.2f, -0.4f), 30.0f);
		spawn(slave);


		slave = createEnt();
		addComp<Base>(slave);
		addComp<Movement>(slave);
		addComp<PhysicsBody>(slave);
		addComp<Collider>(slave, Collider(scaleLegs, Form::RECTANGLE));
		addComp<Draw>(slave, Draw(Vec4(0, 0, 0, 1), scaleLegs, 0.6f, Form::RECTANGLE));
		linkBase(slave, player, Vec2(-0.2f, -0.4f), -30.0f);
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

		int num = 33;
		Vec2 scale = Vec2(0.05f, 0.05f);
		Collider trashCollider = Collider(scale, Form::RECTANGLE);
		Draw trashDraw = Draw(Vec4(1.0f, 1.0f, 1.0f, 1), scale, 0.5f, Form::RECTANGLE, true);
		PhysicsBody trashSolidBody(0.9f, 1.0f, calcMomentOfIntertia(1,scale), 10.0f);
		for (int i = 0; i < num; i++) {
			if (i % 2) {
				//trashCollider.form = Form::CIRCLE;
				//trashDraw.form = Form::CIRCLE;
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
			addComp<Health>(trash, Health(100));
			addComp<TextureRef>(trash, TextureRef("Dir.png"));
			spawn(trash);
			
			auto trashAss = createEnt();
			auto cmps = viewComps(trashAss);
			cmps.add<Base>();
			cmps.add<Movement>();
			auto coll = trashCollider;
			coll.form = Form::CIRCLE;
			cmps.add<Coll>(coll);
			cmps.add<PhysicsBody>();
			auto draw = trashDraw;
			draw.form = Form::CIRCLE;
			cmps.add<Draw>(draw);
			cmps.add<TexRef>(TextureRef("Dir.png"));
			linkBase(trashAss, trash, Vec2(0, 0.02f), 0);
			spawn(trashAss);

			trashAss = createEnt();
			auto cmps2 = viewComps(trashAss);
			cmps2.add<Base>();
			cmps2.add<Movement>();
			coll = trashCollider;
			coll.form = Form::CIRCLE;
			cmps2.add<Coll>(coll);
			cmps2.add<PhysicsBody>();
			draw = trashDraw;
			draw.form = Form::CIRCLE;
			cmps2.add<Draw>(draw);
			cmps2.add<TexRef>(TextureRef("Dir.png"));
			linkBase(trashAss, trash, Vec2(0, -0.02f), 0);
			spawn(trashAss);
		}

		int num2 = 0;
		Vec2 scale2 = Vec2(0.04f, 0.04f);
		Collider trashCollider2 = Collider(scale2, Form::RECTANGLE);
		Draw trashDraw2 = Draw(Vec4(1.0f, 1.0f, 1.0f, 1), scale2, 0.5f, Form::RECTANGLE, true);
		PhysicsBody trashSolidBody2(0.9f, 1'000'000'000'000'000.0f, calcMomentOfIntertia(1, scale), 10.0f);
		for (int i = 0; i < num2; i++) {

			Vec2 position2 = { static_cast<float>(rand() % 1000 / 10.0f - 50.0f) * 4.6f, static_cast<float>(rand() % 1000 / 10.0f - 50.0f) * 4.6f };
			auto trash2 = createEnt();
			addComp<Base>(trash2, Base(position2, RotaVec2(0)));
			addComp<Collider>(trash2, trashCollider2);
			addComp<PhysicsBody>(trash2, trashSolidBody2);
			spawn(trash2);
		}
	}
}
