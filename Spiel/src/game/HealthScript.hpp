#pragma once

#include "Game.hpp"

void onHealthRemCallback(EntityHandleIndex me, Health& data);

void healthScript(EntityHandle me, Health& data, float deltaTime);