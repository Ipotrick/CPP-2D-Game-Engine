#include "LoadRenderTestMap.hpp"

void loadRenderTestMap(float depthRed)
{
	auto redBox = Game::world.create();
	Game::world.addComp(redBox, Transform({ 0, 2 }));
	Game::world.addComp<Collider>(redBox, Collider({ 1,1 }, Form::Rectangle, false));
	Game::world.addComp<Draw>(redBox, Draw({ 1, 0, 0, 1 }, { 1, 1 }, depthRed, Form::Rectangle));
	Game::world.spawn(redBox);

	auto blueBox = Game::world.create();
	Game::world.addComp(blueBox, Transform({0,1}));
	Game::world.addComp(blueBox, Collider({ 1,1 }, Form::Rectangle, false));
	Game::world.addComp(blueBox, Draw({ 0, 0, 1, 1 }, { 1, 1 }, 0.4f, Form::Rectangle));
	Game::world.spawn(blueBox);
}