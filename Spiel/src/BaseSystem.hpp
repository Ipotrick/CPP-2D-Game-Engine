#include "CoreSystem.hpp"

class BaseSystem {
public:
	inline void execute(World& world) {
		for (auto entity : world.index_view<Base>()) {
			if (!world.hasntComp<Movement>(entity) || world.didStaticsChange())	// only update static entites when statics changed
			{
				auto& base = world.getComp<Base>(entity);
				base.rotaVec = RotaVec2(base.rotation);
			}
		}
	}
};