#pragma once

#include "Game.hpp"

void movementScript(Game& game, EntityHandle entity, Transform& t, Movement& m, float deltaTime);
void movementScriptNarrow(Game& game, EntityHandle entity);