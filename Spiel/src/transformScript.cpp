#include "transformScript.hpp"

void transformScript(EntityHandle entity, Transform& t)
{
	t.rotaVec = RotaVec2(t.rotation);
}
