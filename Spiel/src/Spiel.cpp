#include "Engine.h"

#include <iostream>

class Spiel : public Engine {
public:
	Spiel() : Engine("Test", 1600, 900) {
		auto size = getWindowSize();
		camera.frustumBend = (vec2(1/ getWindowAspectRatio(), 1));
	}

	void create() override {
		world.entities.reserve(10000);
		camera.zoom = 1 / 5.0f;

		vec2 scaleEnt = { 0.4, 0.8 };
		auto entC = Entity(Drawable(vec2(0, 0), 0.6, scaleEnt, vec4(0.0, 0.0, 0.0, 1.0), Drawable::Form::RECTANGLE), Collidable(scaleEnt, Collidable::Form::RECTANGLE, 0.99f, true, 10.0f, vec2(0,0)));
		entC.rotation = 45.0f;
		world.spawnEntity(entC);
		controlledEntID = world.entities.at(0).getId();


		vec2 scalePortal = { 4,4 };
		Entity attractor = Entity(Drawable(vec2(-2, 0), 0.4, scalePortal, vec4(1, 0, 0, 1.0), Drawable::Form::CIRCLE), Collidable(scalePortal, Collidable::Form::CIRCLE, 0.9f, true, 100.0f, vec2(0, 0)));
		attractor.solid = false;
		world.spawnEntity(attractor);
		attractorID = world.entities.at(1).getId();

		Entity pusher = Entity(Drawable(vec2(2, 0), 0.4, scalePortal, vec4(0, 0, 1, 1.0), Drawable::Form::CIRCLE), Collidable(scalePortal, Collidable::Form::CIRCLE, 0.9f, true, 100.0f, vec2(0, 0)));
		pusher.solid = false;
		world.spawnEntity(pusher);
		pusherID = world.entities.at(2).getId();

		world.spawnEntity(Entity(Drawable(vec2(-5, 0), 0.5, vec2(0.4, 10), vec4(0.0, 0.0, 0.0, 1.0)),
			Collidable(vec2(0.2, 10), Collidable::Form::RECTANGLE, 1.0f, false, 100000000000000.0f)));
		world.spawnEntity(Entity(Drawable(vec2(5, 0), 0.5, vec2(0.4, 10), vec4(0.0, 0.0, 0.0, 1.0)),
			Collidable(vec2(0.2, 10), Collidable::Form::RECTANGLE, 1.0f, false, 100000000000000.0f)));
		world.spawnEntity(Entity(Drawable(vec2(0, -5), 0.5, vec2(10, 0.4), vec4(0.0, 0.0, 0.0, 1.0)),
			Collidable(vec2(10, 0.2), Collidable::Form::RECTANGLE, 1.0f, false, 100000000000000.0f)));
		world.spawnEntity(Entity(Drawable(vec2(0, 5), 0.5, vec2(10, 0.4), vec4(0.0, 0.0, 0.0, 1.0)),
			Collidable(vec2(10, 0.2), Collidable::Form::RECTANGLE, 1.0f, false, 100000000000000.0f)));

		int num = 20000;

		for (int i = 0; i < num; i++) {
			vec2 pos = { static_cast<float>(rand() % 1000 / 500.0f - 1.0f)*5, static_cast<float>(rand() % 1000 / 500.0f - 1.0f)*5 };
			vec2 scale = vec2(0.04,0.04);
			//vec2 vel = { static_cast<float>(rand() % 1000 / 500.0f - 1.0f)*0.1f, static_cast<float>(rand() % 1000 / 500.0f - 1.0f) * 0.1f };
			vec2 vel = {0,0};

			Drawable::Form formD;
			Collidable::Form formC;
			if (true) {
				formD = Drawable::Form::CIRCLE;
				formC = Collidable::Form::CIRCLE;
			}
			auto newEnt = Entity(Drawable(pos, 0.5, scale, vec4(0, 0, 0, 1.0), formD),
			Collidable(scale, formC, 0.0f, true, 1, vel));
			world.spawnEntity(newEnt);
		}
		world.entities[0].health = 400;
	}

	void update(World& world, float dTime) override {
		
		submitDrawableWindowSpace(Drawable(vec2(0, 0), 0, vec2(2, 2), vec4(1, 1, 1, 1), Drawable::Form::RECTANGLE, 0.0f));

		std::cout << getPerfInfo(4) << std::endl;
		
		if (keyPressed(KEY::LEFT_ALT) && keyPressed(KEY::F4)) {
			quit();
		}

		auto controlledEnt = world.getEntityPtr(controlledEntID);
		auto attractor = world.getEntityPtr(attractorID);
		auto pusher = world.getEntityPtr(pusherID);
		float acceleration = 1.0f;
		float minDist = -attractor->getRadius() + 0.2f;

		auto [begin, end] = getCollisionInfos(attractorID);
		if (begin != end) {
			for (auto iter = begin; iter != end; iter++) {
				auto otherPtr = world.getEntityPtr(iter->idB);
				otherPtr->acceleration += normalize(attractor->getPos() - otherPtr->getPos()) * acceleration * powf((iter->clippingDist / attractor->getRadius()), 2);
				if (iter->clippingDist < minDist) {
					otherPtr->position += pusher->position - attractor->position;
				}
			}
		}

		auto [begin2, end2] = getCollisionInfos(pusherID);
		if (begin2 != end2) {
			for (auto iter = begin2; iter != end2; iter++) {
				auto otherPtr = world.getEntityPtr(iter->idB);
				otherPtr->acceleration += normalize(pusher->getPos() - otherPtr->getPos()) * -acceleration * powf((iter->clippingDist / pusher->getRadius()), 2);
				
			}
		}

		//kills entities one contact
		/*for (size_t i = 0; i < world.entities.size(); i++)
		{
			if (world.entities[i].isCollided() == true)
			{
				int hp{ world.entities[i].getHealth() };
				world.entities[i].health = hp - 1;
			}
			if (world.entities[i].getHealth() <= 0)
			{
				world.despawn(world.entities[i]);
			}
		}*/

		//std::cout << "pos: " << controlledEnt->position << std::endl;
		//std::cout << "vel: " << controlledEnt->velocity << std::endl;
		if (controlledEnt != nullptr) {
			if (keyPressed(KEY::W)) {
				controlledEnt->acceleration.y += 10.0f;
			}
			if (keyPressed(KEY::A)) {
				controlledEnt->acceleration.x -= 10.0f;
			}
			if (keyPressed(KEY::S)) {
				controlledEnt->acceleration.y -= 10.0f;
			}
			if (keyPressed(KEY::D)) {
				controlledEnt->acceleration.x += 10.0f;
			}
			if (keyPressed(KEY::Q)) {
				controlledEnt->rotation += 50.0f * getDeltaTime();
			}
			if (keyPressed(KEY::E)) {
				controlledEnt->rotation -= 50.0f * getDeltaTime();
			}
		}
		
		/*for (Entity& ent : world.entities) {
			if (ent.isDynamic() && ent.getId() != controlledEntID) {
				ent.hitboxSize *= 1 + (0.1f * getDeltaTime());
				ent.mass *= 1 + (0.1f * getDeltaTime());
				ent.scale *= 1 + (0.1f * getDeltaTime());

				auto [begin, end] = getCollisionInfos(ent);

				if (ent.hitboxSize.x > 0.05 && ent.hitboxSize.y > 0.05) {
					for (auto iter = begin; iter != end; iter++) {
						ent.hitboxSize *= 0.75f;
						ent.mass *= 0.75;
						ent.scale *= 0.75;
						break;
					}
				}
			}
		}*/

		if (keyPressed(KEY::UP)) {
			camera.position -= rotate(vec2(0.0f, -5.0f), camera.rotation) * getDeltaTime();
		}
		if (keyPressed(KEY::LEFT)) {
			camera.position -= rotate(vec2(5.0f, 0.0f), camera.rotation) * getDeltaTime();
		}
		if (keyPressed(KEY::DOWN)) {
			camera.position -= rotate(vec2(0.0f, 5.0f), camera.rotation)* getDeltaTime();
		}
		if (keyPressed(KEY::RIGHT)) {
			camera.position -= rotate(vec2(-5.0f, 0.0f), camera.rotation)* getDeltaTime();
		}
		if (keyPressed(KEY::NP_ADD)) {
			camera.zoom *= 1.0f + (1.0f * getDeltaTime());
		}
		if (keyPressed(KEY::NP_SUBTRACT)) {
			camera.zoom *= 1.0f - (1.0f * getDeltaTime());
		}
		if (keyPressed(KEY::NP_0)) {
			camera.rotation = 0.0f;
			camera.position = { 4.5f, 4.5f };
			camera.zoom = 1 / 5.0f;
		}
		
	}

	void destroy() override {}

public:
	uint32_t controlledEntID;
	uint32_t attractorID;
	uint32_t pusherID;
};

int main() {
	Spiel spiel;
	spiel.run();
}