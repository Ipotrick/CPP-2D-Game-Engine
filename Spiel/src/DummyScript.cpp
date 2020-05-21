#include "DummyScript.h"

void DummyScript::script(entity_id me, Dummy& data, float deltaTime) {
	assert(engine.world.exists(me));
	World& world = engine.world;

	auto player = world.getIndex(data.player_id);
	auto compsPlayer = world.viewComps(player);
	auto compsMe = world.viewComps(me);
	float distance = length(compsMe.get<Base>().position - compsPlayer.get<Base>().position);
	//std::cout << "distanz " << distance << std::endl;

	//follow the player script
	float const maxDecel = 30.0f;
	if (distance < 2.3 && length(compsMe.get<Movement>().velocity) > 0)
	{
		Vec2 movDir = normalize(compsMe.get<Movement>().velocity);
		float movAbs = length(compsMe.get<Movement>().velocity);
		movAbs = std::max(0.0f, movAbs - maxDecel * deltaTime);
		compsMe.get<Movement>().velocity = movDir * movAbs;
	}
	if(distance > 2.3)
	{
		Vec2 playerDirec = normalize(Vec2(compsPlayer.get<Base>().position.x - compsMe.get<Base>().position.x, compsPlayer.get<Base>().position.y - compsMe.get<Base>().position.y));
		compsMe.get<Movement>().velocity = playerDirec * 10;
	}
	if (distance > 15)
	{
		compsMe.get<Base>().position = compsPlayer.get<Base>().position - Vec2(1, 1);
	}

	//setting color according to health
	if (compsMe.get<Health>().curHealth < compsMe.get<Health>().maxHealth *0.75)
	{
		compsMe.get<Draw>().color = Vec4(1.f, 0.7f, 0.7f, 1);
	}
	if (compsMe.get<Health>().curHealth < compsMe.get<Health>().maxHealth *0.5)
	{
		compsMe.get<Draw>().color = Vec4(1.f, 0.5f, 0.5f, 1);
	}
	if (compsMe.get<Health>().curHealth < compsMe.get<Health>().maxHealth * 0.25)
	{
		compsMe.get<Draw>().color = Vec4(1.f, 0.2f, 0.2f, 1);
	}
	if (compsMe.get<Health>().curHealth < compsMe.get<Health>().maxHealth * 0.10)
	{
		compsMe.get<Draw>().color = Vec4(1.f, 0.0f, 0.0f, 1);
	}
}