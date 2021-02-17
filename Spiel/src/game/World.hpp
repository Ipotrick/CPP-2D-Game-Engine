#pragma once

#include "EngineConfig.hpp"
#include "../engine/entity/EntityComponentManager.hpp"
#include "../engine/collision/CoreSystemUniforms.hpp"

class World : public EntityComponentManager<ENGINE_COMPONENT_LIST> {
	friend class YAMLEntitySerializer;
	friend class YAMLWorldSerializer;
public:
	PhysicsUniforms physics;
private:
};