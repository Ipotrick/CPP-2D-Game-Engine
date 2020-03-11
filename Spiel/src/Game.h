#pragma once

#include <iostream>

#include "Engine.h"
#include "GameComponents.h"

class Dog;

class Game : public Engine {
public:
	Game();

	void create() override;

	void update(World& world, float dTime) override;

	void destroy() override {}

public:
	std::vector<void*> componentControllers;
	CompControllerMortal mortalController;
	CompControllerPlayer playerController;
	CompControllerBullet bulletController;
	uint32_t controlledEntID;
	uint32_t attractorID;
	uint32_t pusherID;
	
};