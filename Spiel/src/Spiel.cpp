#include "Engine.h"

#include <iostream>

class Spiel : public Engine {
public:
	Spiel() : Engine("Test", 1000, 1000) {}

	void update(World& world, double dTime) override {
		std::cout << getPerfInfo(2) << std::endl << "iteration: " << getIteration() << std::endl;
		world.entities.at(0).position.x = sin(getIteration() / 360.0) /2.0;
		world.entities.at(0).position.y = cos(getIteration() / 360.0) /2.0;
		world.entities.at(0).rotation = -(getIteration() / 3.14*0.5);
	}

	void create() override {
		world.spawnEntity(Entity(Drawable(), Collidable()));
		world.entities.at(0).scale = (vec2(0.2, 0.2));
	}

	void destroy() override {}
};

int main() {
	Spiel spiel;
	spiel.run();
}