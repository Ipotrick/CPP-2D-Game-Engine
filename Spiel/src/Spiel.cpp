#include "Engine.h"

#include <iostream>

class Spiel : public Engine {
public:
	Spiel() : Engine("Test", 1000, 1000) {}

	void create() override {
		world.spawnEntity(Entity(Drawable(vec2(0.0, 1.0), 0.5, vec2(0.2, 0.2), vec4(0.4, 0.4, 0.6, 1.0)), Collidable(vec2(), Collidable::Form::RECTANGLE, 1.0f, true, 1.0f)));
		for (int i = 0; i < 10; i++) {
			for (int j = 0; j < 10; j++) {
				vec4 color = ((i+j)%2 == 0)? vec4(1.0,1.0,1.0,1.0) : vec4(0.0,0.0,0.0,1.0);
				world.spawnEntity(Entity(Drawable(vec2(i, j), 0, vec2(1, 1), color), Collidable(vec2(), Collidable::Form::RECTANGLE, 1.0f, false, 1.0f)));
			}
		}
		
	}

	void update(World& world, double dTime) override {
		std::cout << getPerfInfo(2) << std::endl << "iteration: " << getIteration() << std::endl;

		for (auto& ent : world.entities) {
			if (ent.isDynamic()) {
				ent.position.x = sin((getIteration() + ent.getId()) / 360.0f) / 2.0f;
				ent.position.y = cos((getIteration() + ent.getId()) / 360.0f) / 2.0f;
				ent.rotation = -((getIteration() + ent.getId()) / 3.14f * 0.5f);
			}
		}
	}

	void destroy() override {}
};

int main() {
	Spiel spiel;
	spiel.run();
}