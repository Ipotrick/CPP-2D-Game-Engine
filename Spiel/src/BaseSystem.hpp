#include "CoreSystem.hpp"

class BaseSystem {
public:
	inline void execute(World& world) {
		for (auto entity : world.entityView<Transform>()) {
			auto& base = world.getComp<Transform>(entity);
			base.rotaVec = RotaVec2(base.rotation);
		}
	}
};