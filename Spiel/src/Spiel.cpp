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

		Quadtree qtree(vec2(-5, -5), vec2(5, 5), 2);
		world.spawnEntity(Entity(Drawable(vec2(4, 4), 0.6, vec2(0.2, 0.2), vec4(1, 0.3, 0.9, 1.0)), Collidable(vec2(0.2, 0.2), Collidable::Form::RECTANGLE, 1.0f, true, 1.0f, vec2(1,1))));
		controlledEntID = world.entities.at(0).getId();


		world.spawnEntity(Entity(Drawable(vec2(4, 3.9), 0.4, vec2(0.2, 0.2), vec4(0.4, 0.4, 0.6, 1.0)), Collidable(vec2(0.2, 0.2), Collidable::Form::RECTANGLE, 1.0f, true, 1.0f, vec2(0, -1))));

		world.spawnEntity(Entity(Drawable(vec2(-5, 0), 0.5, vec2(0.2, 10), vec4(0.5, 0.8, 0.2, 1.0)), 
			Collidable(vec2(0.2, 10), Collidable::Form::RECTANGLE, 1.0f, false)));
		world.spawnEntity(Entity(Drawable(vec2(5, 0), 0.5, vec2(0.2, 10), vec4(0.5, 0.8, 0.2, 1.0)),
			Collidable(vec2(0.2, 10), Collidable::Form::RECTANGLE, 1.0f, false)));
		world.spawnEntity(Entity(Drawable(vec2(0, -5), 0.5, vec2(10, 0.2), vec4(0.5, 0.8, 0.2, 1.0)),
			Collidable(vec2(10, 0.2), Collidable::Form::RECTANGLE, 1.0f, false)));
		world.spawnEntity(Entity(Drawable(vec2(0, 5), 0.5, vec2(10, 0.2), vec4(0.5, 0.8, 0.2, 1.0)),
			Collidable(vec2(10, 0.2), Collidable::Form::RECTANGLE, 1.0f, false)));

		for (auto el : world.entities) qtree.insert(&el);

		int num = 2000;

		for (int i = 0; i < num; i++) {
			vec2 pos = { static_cast<float>(rand() % 1000 / 500.0f - 1.0f)*5, static_cast<float>(rand() % 1000 / 500.0f - 1.0f)*5 };
			vec2 scale = { 0.2,0.2 };
			vec2 vel = { static_cast<float>(rand() % 1000 / 500.0f - 1.0f)*0.1f, static_cast<float>(rand() % 1000 / 500.0f - 1.0f) * 0.1f };

			auto newEnt = Entity(Drawable(pos, 0.5, scale, vec4(0.4, 0.4, 0.6, 1.0)),
			Collidable(scale, Collidable::Form::RECTANGLE, 1.0f, true, 1, vel));
			world.spawnEntity(newEnt);
		}

	}

	void update(World& world, float dTime) override {

		std::cout << getPerfInfo(4) << std::endl;

		auto controlledEnt = world.getEntityPtr(controlledEntID);
		if (controlledEnt != nullptr) {
			if (keyPressed(KEY::W)) {
				controlledEnt->velocity.y += -1.0f * getDeltaTime();
			}
			if (keyPressed(KEY::A)) {
				controlledEnt->velocity.x += 1.0f * getDeltaTime();
			}
			if (keyPressed(KEY::S)) {
				controlledEnt->velocity.y += 1.0f * getDeltaTime();
			}
			if (keyPressed(KEY::D)) {
				controlledEnt->velocity.x += -1.0f * getDeltaTime();
			}
			if (keyPressed(KEY::Q)) {
				controlledEnt->rotation += 50.0f * getDeltaTime();
			}
			if (keyPressed(KEY::E)) {
				controlledEnt->rotation -= 50.0f * getDeltaTime();
			}
		}
		
		if (controlledEnt != nullptr) {
			auto [begin, end] = getCollisionInfos(*controlledEnt);

			for (auto iter = begin; iter != end; iter++) {
			}
		}

		
		for (Entity& ent : world.entities) {
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
		}

		if (keyPressed(KEY::UP)) {
			camera.position += rotate(vec2(0.0f, -5.0f), camera.rotation) * getDeltaTime();
		}
		if (keyPressed(KEY::LEFT)) {
			camera.position += rotate(vec2(5.0f, 0.0f), camera.rotation) * getDeltaTime();
		}
		if (keyPressed(KEY::DOWN)) {
			camera.position += rotate(vec2(0.0f, 5.0f), camera.rotation)* getDeltaTime();
		}
		if (keyPressed(KEY::RIGHT)) {
			camera.position += rotate(vec2(-5.0f, 0.0f), camera.rotation)* getDeltaTime();
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