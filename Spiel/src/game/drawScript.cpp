#include "drawScript.hpp"
#include "LayerConstants.hpp"

void drawScript(EntityHandle entity, const Transform& t, const Draw& d)
{
	if (Game::world.hasComp<TextureRef2>(entity)) {
		TextureRef2& texRef = Game::world.getComp<TextureRef2>(entity);
		if (!texRef.good()) {
			// if a TexRef component was created without the renderer, it will be replaced here:
			EngineCore::renderer.validateTextureRef(texRef);
		}
		EngineCore::renderer.submit(Drawable(0, t.position, d.drawingPrio, d.scale, d.color, d.form, t.rotaVec, RenderSpace::WorldSpace, texRef.makeSmall()), LAYER_WORLD_MIDGROUND);
	}
	else {
		EngineCore::renderer.submit(Drawable(0, t.position, d.drawingPrio, d.scale, d.color, d.form, t.rotaVec, RenderSpace::WorldSpace), LAYER_WORLD_MIDGROUND);
	}
}
