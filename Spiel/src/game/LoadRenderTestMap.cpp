#include "LoadRenderTestMap.hpp"

void loadRenderTestMap(Game& game, float depthRed)
{
	auto redBox = game.world.create();
	game.world.addComp(redBox, Transform({ 0, 2 }));
	game.world.addComp<Collider>(redBox, Collider({ 1,1 }, Form::Rectangle, false));
	game.world.addComp<Draw>(redBox, Draw({ 1, 0, 0, 1 }, { 1, 1 }, depthRed, Form::Rectangle));
	game.world.spawn(redBox);

	auto blueBox = game.world.create();
	game.world.addComp(blueBox, Transform({0,1}));
	game.world.addComp(blueBox, Collider({ 1,1 }, Form::Rectangle, false));
	game.world.addComp(blueBox, Draw({ 0, 0, 1, 1 }, { 1, 1 }, 0.4f, Form::Rectangle));
	game.world.spawn(blueBox);
}