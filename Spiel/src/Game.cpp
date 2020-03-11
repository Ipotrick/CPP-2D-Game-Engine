#include "Game.h"

Game::Game() : Engine("Test", 1600, 900), componentControllers{}, mortalController(*this, componentControllers), playerController(*this, componentControllers)
{
	componentControllers.push_back(&mortalController);
	componentControllers.push_back(&playerController);

	auto size = getWindowSize();
	camera.frustumBend = (vec2(1 / getWindowAspectRatio(), 1));
}

void Game::create() {
	camera.zoom = 1 / 5.0f;

	vec2 scaleEnt = { 0.4, 0.8 };
	auto entC = Entity(Drawable(vec2(0, 0), 0.61, scaleEnt, vec4(0.0, 0.0, 0.0, 1), Drawable::Form::RECTANGLE), Collidable(scaleEnt, Collidable::Form::RECTANGLE, 0.99f, true, 60.0f, vec2(0, 0)));
	entC.rotation = 45.0f;
	world.spawnEntity(entC);
	controlledEntID = world.latestID;
	playerController.registerEntity(CompDataPlayer(controlledEntID));

	vec2 scalePortal = { 14, 14 };
	Entity attractor = Entity(Drawable(vec2(-2, 0), 0.41, scalePortal, vec4(1, 0, 0, 0.5), Drawable::Form::CIRCLE), Collidable(scalePortal, Collidable::Form::CIRCLE, 0.9f, true, 100.0f, vec2(0, 0)));
	attractor.solid = false;
	world.spawnEntity(attractor);
	attractorID = world.latestID;

	Entity pusher = Entity(Drawable(vec2(2, 0), 0.4, scalePortal, vec4(0, 0, 1, 0.5), Drawable::Form::CIRCLE), Collidable(scalePortal, Collidable::Form::CIRCLE, 0.9f, true, 100.0f, vec2(0, 0)));
	pusher.solid = false;
	world.spawnEntity(pusher);
	pusherID = world.latestID;

	world.spawnEntity(Entity(Drawable(vec2(-5, 0), 0.5, vec2(0.4, 10), vec4(0.0, 0.0, 0.0, 1.0)),
		Collidable(vec2(0.4, 10), Collidable::Form::RECTANGLE, 1.0f, false, 100000000000000.0f)));
	mortalController.registerEntity(CompDataMortal(world.latestID, 0, 1, -1));
	world.spawnEntity(Entity(Drawable(vec2(5, 0), 0.5, vec2(0.4, 10), vec4(0.0, 0.0, 0.0, 1.0)),
		Collidable(vec2(0.4, 10), Collidable::Form::RECTANGLE, 1.0f, false, 100000000000000.0f)));
	mortalController.registerEntity(CompDataMortal(world.latestID, 0, 1, -1));
	world.spawnEntity(Entity(Drawable(vec2(0, -5), 0.5, vec2(10, 0.4), vec4(0.0, 0.0, 0.0, 1.0)),
		Collidable(vec2(10, 0.4), Collidable::Form::RECTANGLE, 1.0f, false, 100000000000000.0f)));
	mortalController.registerEntity(CompDataMortal(world.latestID, 0, 1, -1));
	world.spawnEntity(Entity(Drawable(vec2(0, 5), 0.5, vec2(10, 0.4), vec4(0.0, 0.0, 0.0, 1.0)),
		Collidable(vec2(10, 0.4), Collidable::Form::RECTANGLE, 1.0f, false, 100000000000000.0f)));
	mortalController.registerEntity(CompDataMortal(world.latestID, 0, 1, -1));

	int num = 5000;

	for (int i = 0; i < num; i++) {
		vec2 pos = { static_cast<float>(rand() % 1000 / 500.0f - 1.0f) * 5, static_cast<float>(rand() % 1000 / 500.0f - 1.0f) * 5 };
		vec2 scale = vec2(0.10, 0.10);
		//vec2 vel = { static_cast<float>(rand() % 1000 / 500.0f - 1.0f)*0.1f, static_cast<float>(rand() % 1000 / 500.0f - 1.0f) * 0.1f };
		vec2 vel = { 0,0 };

		Drawable::Form formD;
		Collidable::Form formC;
		if (true) {
			formD = Drawable::Form::CIRCLE;
			formC = Collidable::Form::CIRCLE;
		}
		auto newEnt = Entity(Drawable(pos, 0.5, scale, vec4(0, 0, 0, 1.0), formD),
			Collidable(scale, formC, 0.0f, true, 1, vel));
		world.spawnEntity(newEnt);
		//mortalController.registerEntity(CompDataMortal(world.entities.at(world.entities.size() - 1).getId(), 100, 1, -1.0f));
	}
}

void Game::update(World& world, float dTime) {
	//take input
	if (keyPressed(KEY::LEFT_ALT) && keyPressed(KEY::F4)) {
		quit();
	}

	playerController.executeScripts(dTime);

	if (keyPressed(KEY::UP)) {
		camera.position -= rotate(vec2(0.0f, -5.0f), camera.rotation) * dTime;
	}
	if (keyPressed(KEY::LEFT)) {
		camera.position -= rotate(vec2(5.0f, 0.0f), camera.rotation) * dTime;
	}
	if (keyPressed(KEY::DOWN)) {
		camera.position -= rotate(vec2(0.0f, 5.0f), camera.rotation) * dTime;
	}
	if (keyPressed(KEY::RIGHT)) {
		camera.position -= rotate(vec2(-5.0f, 0.0f), camera.rotation) * dTime;
	}
	if (keyPressed(KEY::NP_ADD)) {
		camera.zoom *= 1.0f + (1.0f * dTime);
	}
	if (keyPressed(KEY::NP_SUBTRACT)) {
		camera.zoom *= 1.0f - (1.0f * dTime);
	}
	if (keyPressed(KEY::NP_0)) {
		camera.rotation = 0.0f;
		camera.position = { 0,0 };
		camera.zoom = 1 / 5.0f;
	}

	// execute all scripts in the beginning after input
	
	mortalController.executeScripts(dTime);
	

	//set backgound
	submitDrawableWindowSpace(Drawable(vec2(0, 0), 0, vec2(2, 2), vec4(1, 1, 1, 1), Drawable::Form::RECTANGLE, 0.0f));

	//display performance statistics
	std::cout << getPerfInfo(4) << std::endl;
	
	LogTimer<> t(std::cout << "attractor");
	auto attractor = world.getEntityPtr(attractorID);
	auto pusher = world.getEntityPtr(pusherID);
	float acceleration = 1.0f;
	float minDist = -attractor->getRadius() + 0.2f;

	auto [begin, end] = getCollisionInfosHash(attractorID);
	if (begin != end) {
		for (auto iter = begin; iter != end; iter++) {
			auto otherPtr = world.getEntityPtr(iter->idB);
			if (otherPtr->isSolid()) {
				otherPtr->acceleration += normalize(attractor->getPos() - otherPtr->getPos()) * acceleration * powf((iter->clippingDist / attractor->getRadius()), 2);
				if (iter->clippingDist < minDist) {
					//otherPtr->position += pusher->position - attractor->position;
					vec2 otherToCenter = (attractor->position + pusher->position)/2.0f - otherPtr->getPos();
					otherPtr->position += otherToCenter * 2;
				}
			}
		}
	}

	auto [begin2, end2] = getCollisionInfosHash(pusherID);
	if (begin2 != end2) {
		for (auto iter = begin2; iter != end2; iter++) {
			auto otherPtr = world.getEntityPtr(iter->idB);
			if (otherPtr->isSolid()) {
				//otherPtr->acceleration += normalize(pusher->getPos() - otherPtr->getPos()) * -acceleration * powf((iter->clippingDist / pusher->getRadius()), 2);
			}
		}
	}
	t.stop();
	
	//clean up component idLists
	auto despawns = world.getDespawnIDs();
	for (auto id : despawns) {
		//deregister ALL entities that are going to be delteted
		mortalController.deregisterEntity(id);
		playerController.deregisterEntity(id);
	}
}