#include "Game.h"

Game::Game() : Engine("Test", 1600, 900)
{

	auto size = getWindowSize();
	camera.frustumBend = (vec2(1 / getWindowAspectRatio(), 1));
}

void Game::create() {
	camera.zoom = 1 / 5.0f;

	std::cout << "sizeof(Entity): " << sizeof(Entity) << std::endl;
	std::cout << "sizeof(Collidable): " << sizeof(Collidable) << std::endl;
	std::cout << "sizeof(Drawable): " << sizeof(Drawable) << std::endl;

	vec2 scaleEnt = { 0.4, 0.8 };
	//auto entC = Entity(Drawable(vec2(0, 0), 0.61, scaleEnt, vec4(0.0, 0.0, 0.0, 1), Drawable::Form::RECTANGLE), Collidable(scaleEnt, Collidable::Form::RECTANGLE, 0.99f, true, 60.0f, vec2(0, 0)));
	auto cEnt = Entity(vec2(0, 0), 0.0f, Collidable(scaleEnt, Collidable::Form::RECTANGLE, true, true,  0.5f, 50.0f, vec2(0,0)));
	auto cDraw = CompDataDrawable(vec4(0, 0, 0, 1), scaleEnt);
	cEnt.rotation = 45.0f;
	world.spawnEntity(cEnt, cDraw);
	controlledEntID = world.getLastID();
	world.playerController.registerEntity(controlledEntID, CompDataPlayer());

	/*
	Entity attractor = Entity(Drawable(vec2(-2, 0), 0.41, scalePortal, vec4(1, 0, 0, 0.5), Drawable::Form::CIRCLE), Collidable(scalePortal, Collidable::Form::CIRCLE, 0.9f, true, 100.0f, vec2(0, 0)));
	attractor.solid = false;
	world.spawnEntity(attractor);
	attractorID = world.lastID;

	Entity pusher = Entity(Drawable(vec2(2, 0), 0.4, scalePortal, vec4(0, 0, 1, 0.5), Drawable::Form::CIRCLE), Collidable(scalePortal, Collidable::Form::CIRCLE, 0.9f, true, 100.0f, vec2(0, 0)));
	pusher.solid = false;
	world.spawnEntity(pusher);
	pusherID = world.lastID;*/

	vec2 scalePortal = { 28, 28 };
	Entity portalC = Entity(vec2(-4, -4), 0, Collidable(scalePortal, Collidable::Form::CIRCLE, false, true));
	CompDataDrawable portalD = CompDataDrawable(vec4(1, 0, 0, 0.5f), scalePortal, 0.49f, CompDataDrawable::Form::CIRCLE);
	world.spawnEntity(portalC, portalD);
	attractorID = world.getLastID();

	portalC.hitboxSize = scalePortal / 16;
	portalD.scale = scalePortal / 16;
	portalC.position = vec2(4, 4);
	portalD.color = vec4(0, 0, 1, 0.5f);
	portalD.drawingPrio = 0.48f;
	world.spawnEntity(portalC, portalD);
	pusherID = world.getLastID();

	Entity wallC = Entity(vec2(0,0), 0, Collidable(vec2(0.4, 10), Collidable::Form::RECTANGLE, true, false, 0.5f,  1'000'000'000'000'000.0f, vec2(0,0)));
	CompDataDrawable wallD = CompDataDrawable(vec4(0, 0, 0, 1), vec2(0.4, 10), 0.5f, CompDataDrawable::Form::RECTANGLE);
	for (int i = 0; i < 4; i++) {
		float rotation = 90.0f * i;
		wallC.position = rotate(vec2(-5.f, 0), rotation);
		wallC.rotation = rotation;
		world.spawnEntity(wallC, wallD);
	}

	int num = 5000;

	vec2 scale = vec2(0.1, 0.1);
	Entity trashEntC = Entity(vec2(0, 0), 0.0f, Collidable(scale, Collidable::Form::CIRCLE, true, true, 0.5f, 1.0f, vec2()));
	CompDataDrawable trashEntD = CompDataDrawable(vec4(0, 0, 0, 1), scale, 0.5f, CompDataDrawable::Form::CIRCLE);
	for (int i = 0; i < num; i++) {
		trashEntC.position = { static_cast<float>(rand() % 1000 / 500.0f - 1.0f) * 5, static_cast<float>(rand() % 1000 / 500.0f - 1.0f) * 5 };

		world.spawnEntity(trashEntC, trashEntD);
		world.mortalController.registerEntity(world.getLastID(), CompDataMortal(100, 0, -1.0f));
	}
}

void Game::update(World& world, float dTime) {
	//take input

	if (keyPressed(KEY::LEFT_ALT) && keyPressed(KEY::F4)) {
		quit();
	}

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
	for (auto& el : world.playerController.componentData) {
		auto* entity = world.getEntityPtr(el.first);
		float exaustVel = 1.0f;
		vec2 exaustSize = vec2(0.06f, 0.06f);
		float exaustamount = 1;
		if (keyPressed(KEY::W)) {
			entity->acceleration += rotate(vec2(0.0f, 2.0f), entity->rotation);
			//for (int i = 0; i < exaustamount; i++) {
			//	world.spawnEntity(Entity(Drawable(entity->getPos() + rotate(vec2(-entity->scale.y, 0) / 3.0f, entity->rotation + 90), entity->drawingPrio - 0.01f, exaustSize, vec4(1, 0, 0, 0.8f), Drawable::Form::CIRCLE, entity->rotation),
			//		Collidable(exaustSize, Collidable::Form::CIRCLE, 0.4f, true, 0.001f, entity->velocity + exaustVel * rotate(vec2(0, 1), entity->rotation))));
			//	world.mortalController.registerEntity(CompDataMortal(engine.world.latestID, -1, 0, 0.3f));
			//}
			//entity->position += vec2(0, 1) * dTime;
		}
		if (keyPressed(KEY::A)) {
			entity->acceleration += rotate(vec2(-10.0f, 0.0f), entity->rotation);
			//entity->position += vec2(-1, 0) * dTime;
		}
		if (keyPressed(KEY::S)) {
			entity->acceleration += rotate(vec2(0.0f, -10.0f), entity->rotation);
			//entity->position += vec2(0, -1) * dTime;
		}
		if (keyPressed(KEY::D)) {
			entity->acceleration += rotate(vec2(10.0f, 0.0f), entity->rotation);
			//entity->position += vec2(1, 0) * dTime;
		}
		if (keyPressed(KEY::Q)) {
			entity->rotation += 200.0f * dTime;
		}
		if (keyPressed(KEY::E)) {
			entity->rotation -= 200.0f * dTime;
		}
		if (keyPressed(KEY::F)) {
			//engine.world.spawnEntity(Entity(Drawable(entity->getPos() + rotate(vec2(entity->scale.y, 0) / 1.8f, entity->rotation + 90), entity->drawingPrio - 0.01f, exaustSize, vec4(0, 0, 1, 0.8f), Drawable::Form::CIRCLE, entity->rotation),
			//	Collidable(exaustSize, Collidable::Form::CIRCLE, 0.4f, true, 0.1f, 10.0f * rotate(vec2(0, 1), entity->rotation))));
			//world.mortalController.registerEntity(CompDataMortal(engine.world.latestID, -1, 10, -1));
			//world.bulletController.registerEntity(CompDataBullet(engine.world.latestID));
		}
	}
	

	//set backgound
	submitDrawableWindowSpace(Drawable(vec2(0, 0), 0, vec2(2, 2), vec4(1, 1, 1, 1), Drawable::Form::RECTANGLE, 0.0f));

	//display performance statistics
	std::cout << getPerfInfo(4) << '\n';
	
	auto attractor = world.getEntityPtr(attractorID);
	auto pusher = world.getEntityPtr(pusherID);
	float acceleration = 1.0f;
	float minDist = -attractor->getRadius() + 0.2f;

	auto [begin, end] = getCollisionInfos(attractorID);
	if (begin != end) {
		for (auto iter = begin; iter != end; iter++) {
			auto otherPtr = world.getEntityPtr(iter->idB);
			if (otherPtr->isSolid() && otherPtr->isDynamic()) {
				otherPtr->acceleration += normalize(attractor->getPos() - otherPtr->getPos()) * acceleration * powf((iter->clippingDist / attractor->getRadius()), 2);
				if (iter->clippingDist < minDist) {
					//otherPtr->position += pusher->position - attractor->position;
					vec2 otherToCenter = (attractor->position + pusher->position)/2.0f - otherPtr->getPos();
					otherPtr->position += otherToCenter * 2;
				}
			}
		}
	}
	auto [begin2, end2] = getCollisionInfos(pusherID);
	if (begin2 != end2) {
		for (auto iter = begin2; iter != end2; iter++) {
			auto otherPtr = world.getEntityPtr(iter->idB);
			if (otherPtr->isSolid()) {
				otherPtr->acceleration += normalize(pusher->getPos() - otherPtr->getPos()) * -acceleration * powf((iter->clippingDist / pusher->getRadius()), 2);
			}
		}
	}
	
}