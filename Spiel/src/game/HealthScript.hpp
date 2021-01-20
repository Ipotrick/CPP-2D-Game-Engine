#pragma once

#include "Game.hpp"

void onHealthRemCallback(EntityHandleIndex me, Health& data);

void healthScript(Game& game, EntityHandle me, Health& data, float deltaTime);