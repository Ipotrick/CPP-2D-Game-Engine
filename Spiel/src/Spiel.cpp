#include "Engine.h"

#include <iostream>

class Spiel : public Engine {
public:
	Spiel() : Engine("Test", 1600, 900) {
		auto size = getWindowSize();
		camera.frustumBend = (vec2(1/ getWindowAspectRatio(), 1));
	}

	void create() override {
		world.spawnEntity(Entity(Drawable(vec2(0.0, 1.0), 0.5, vec2(0.2, 0.2), vec4(0.4, 0.4, 0.6, 1.0)), Collidable(vec2(), Collidable::Form::RECTANGLE, 1.0f, true, 1.0f)));
		for (int i = 0; i < 10; i++) {
			for (int j = 0; j < 10; j++) {
				vec4 color = ((i+j)%2 == 0)? vec4(1.0,1.0,1.0,1.0) : vec4(0.0,0.0,0.0,1.0);
				world.spawnEntity(Entity(Drawable(vec2(i, j), 0, vec2(1, 1), color), Collidable(vec2(), Collidable::Form::RECTANGLE, 1.0f, false, 1.0f)));
			}
		}
		camera.position = vec2(4.5, 4.5);
		camera.zoom = 1 / 5.0f;
	}

	void update(World& world, double dTime) override {
		std::cout << getPerfInfo(2) << std::endl << "iteration: " << getIteration() << std::endl;

		for (auto& ent : world.entities) {
			if (ent.isDynamic()) {
				ent.position.x = sin((getIteration() + ent.getId()) / 360.0f) / 2.0f + 4.5f;
				ent.position.y = cos((getIteration() + ent.getId()) / 360.0f) / 2.0f + 4.5f;
				ent.rotation = -((getIteration() + ent.getId()) / 3.14f * 0.5f);
			}
		}

		if (keyPressed(KEY::W)) {
			camera.position += rotate(vec2(0.0, -5.0), camera.rotation) * getDeltaTime();
		}
		if (keyPressed(KEY::A)) {
			camera.position += rotate(vec2(5.0, 0.0), camera.rotation) * getDeltaTime();
		}
		if (keyPressed(KEY::S)) {
			camera.position += rotate(vec2(0.0, 5.0), camera.rotation)* getDeltaTime();
		}
		if (keyPressed(KEY::D)) {
			camera.position += rotate(vec2(-5.0, 0.0), camera.rotation)* getDeltaTime();
		}
		if (keyPressed(KEY::NP_ADD)) {
			camera.zoom *= 1.0 + (1.0 * getDeltaTime());
		}
		if (keyPressed(KEY::NP_SUBTRACT)) {
			camera.zoom *= 1.0 - (1.0 * getDeltaTime());
		}
		if (keyPressed(KEY::Q)) {
			camera.rotation += 50.0 * getDeltaTime();
		}
		if (keyPressed(KEY::E)) {
			camera.rotation -= 50.0 * getDeltaTime();
		}
		if (keyPressed(KEY::NP_0)) {
			camera.rotation = 0.0f;
			camera.position = { 4.5f, 4.5f };
			camera.zoom = 1 / 5.0f;
		}
	}

	void destroy() override {}
};

int main() {
	Spiel spiel;
	spiel.run();
}