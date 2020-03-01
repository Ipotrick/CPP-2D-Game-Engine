#pragma once

#include "Entity.h"
#include <vector>

class World {
public:
	World() {}
	std::vector<Entity> entities;
};