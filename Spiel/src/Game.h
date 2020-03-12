#pragma once

#include <iostream>

#include "Engine.h"
#include "GameComponents.h"

class Game : public Engine {
public:
	Game();

	void create() override;

	void update(World& world, float dTime) override;

	void destroy() override {}

public:
	
	uint32_t controlledEntID;
	uint32_t attractorID;
	uint32_t pusherID;
	
};