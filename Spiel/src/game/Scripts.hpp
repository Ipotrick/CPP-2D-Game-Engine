#pragma once

#include "Game.hpp"

void ageScript(Game& game, EntityHandle id, Age& data, float deltaTime);

void bulletScript(Game& game, EntityHandle me, Bullet& data, float deltaTime);

void drawScript(Game& game, EntityHandle entity, const Transform& t, const Draw& d);
