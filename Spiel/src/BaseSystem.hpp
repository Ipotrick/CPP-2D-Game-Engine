#include "CoreSystem.hpp"

class BaseSystem {
public:
	inline void execute(World& world) {
		for (auto entity : world.entityView<Base>()) {
			auto& base = world.getComp<Base>(entity);
			base.rotaVec = RotaVec2(base.rotation);
		}
	}
};