#include "Engine.h"

#include <iostream>

class Spiel : public Engine {
public:
	Spiel() : Engine("Test",1,1) {}
	void update(World world, double dTime) override {
		std::cout << getPerfInfo(1) << std::endl << "iteration: " << getIteration() << std::endl;
	}
	void create() override {}
	void destroy() override {}
};

int main()
{
	Spiel spiel;
	spiel.run();
}