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

		vec2 scaleEnt = { 0.5,0.5 };
		world.spawnEntity(Entity(Drawable(vec2(4, 4), 0.6, scaleEnt, vec4(1, 0.3, 0.9, 0.5), Drawable::Form::RECTANGLE), Collidable(scaleEnt, Collidable::Form::RECTANGLE, 0.9f, true, 39.0f, vec2(1.7,1.7))));
		controlledEntID = world.entities.at(0).getId();

		world.spawnEntity(Entity(Drawable(vec2(4.5, 2.5), 0.4, vec2(0.2, 0.2), vec4(0.4, 0.4, 0.6, 1.0), Drawable::Form::RECTANGLE), Collidable(vec2(0.2, 0.2), Collidable::Form::RECTANGLE, 0.9f, true, 9.0f, vec2(0, 0))));

		world.spawnEntity(Entity(Drawable(vec2(-5, 0), 0.5, vec2(0.2, 10), vec4(0.8, 0.8, 0.2, 1.0)), 
			Collidable(vec2(0.2, 10), Collidable::Form::RECTANGLE, 1.0f, false, 100000000000000.0f)));
		world.spawnEntity(Entity(Drawable(vec2(5, 0), 0.5, vec2(0.2, 10), vec4(0.5, 0.8, 0.2, 1.0)),
			Collidable(vec2(0.2, 10), Collidable::Form::RECTANGLE, 1.0f, false, 100000000000000.0f)));
		world.spawnEntity(Entity(Drawable(vec2(0, -5), 0.5, vec2(10, 0.2), vec4(0.5, 0.8, 0.2, 1.0)),
			Collidable(vec2(10, 0.2), Collidable::Form::RECTANGLE, 1.0f, false, 100000000000000.0f)));
		world.spawnEntity(Entity(Drawable(vec2(0, 5), 0.5, vec2(10, 0.2), vec4(0.5, 0.8, 0.2, 1.0)),
			Collidable(vec2(10, 0.2), Collidable::Form::RECTANGLE, 1.0f, false, 100000000000000.0f)));


		int num = 1000;

		for (int i = 0; i < num; i++) {
			vec2 pos = { static_cast<float>(rand() % 1000 / 500.0f - 1.0f)*5, static_cast<float>(rand() % 1000 / 500.0f - 1.0f)*5 };
			vec2 scale = { 0.1,0.1 };
			//vec2 vel = { static_cast<float>(rand() % 1000 / 500.0f - 1.0f)*0.1f, static_cast<float>(rand() % 1000 / 500.0f - 1.0f) * 0.1f };
			vec2 vel = {0,0};

			auto newEnt = Entity(Drawable(pos, 0.5, scale, vec4(0.4, 0.4, 0.6, 1.0), Drawable::Form::CIRCLE),
			Collidable(scale, Collidable::Form::CIRCLE, 0.1f, true, 4, vel));
			world.spawnEntity(newEnt);
		}

		world.entities[0].health = 400;

	}

	void update(World& world, float dTime) override {
		
		std::cout << getPerfInfo(4) << std::endl;
		
		if (keyPressed(KEY::LEFT_ALT) && keyPressed(KEY::F4)) {
			quit();
		}

		auto controlledEnt = world.getEntityPtr(controlledEntID);

		//kills entities one contact
		/*for (size_t i = 0; i < world.entities.size(); i++)
		{
			if (world.entities[i].isCollided() == true)
			{
				int hp{ world.entities[i].getHealth() };
				world.entities[i].health = hp - 100;
			}
			if (world.entities[i].getHealth() <= 0)
			{
				world.despawn(world.entities[i]);
			}
		}*/

		if (controlledEnt != nullptr) {
			if (keyPressed(KEY::W)) {
				controlledEnt->velocity.y += 1.0f * getDeltaTime();
			}
			if (keyPressed(KEY::A)) {
				controlledEnt->velocity.x -= 1.0f * getDeltaTime();
			}
			if (keyPressed(KEY::S)) {
				controlledEnt->velocity.y -= 1.0f * getDeltaTime();
			}
			if (keyPressed(KEY::D)) {
				controlledEnt->velocity.x += 1.0f * getDeltaTime();
			}
			if (keyPressed(KEY::Q)) {
				controlledEnt->rotation += 50.0f * getDeltaTime();
			}
			if (keyPressed(KEY::E)) {
				controlledEnt->rotation -= 50.0f * getDeltaTime();
			}
		}

		std::cout << std::endl;
		
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
};

int main() {
	Spiel spiel;
	spiel.run();
}