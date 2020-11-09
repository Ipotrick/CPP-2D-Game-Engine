#include "drawScript.hpp"

void drawScript(EntityHandle entity, const Transform& t, const Draw& d)
{
	if (Engine::world.hasComp<TextureRef2>(entity)) {
		TextureRef2& texRef = Engine::world.getComp<TextureRef2>(entity);
		if (!texRef.good()) {
			// if a TexRef component was created without the renderer, it will be replaced here:
			Engine::renderer.validateTextureRef(texRef);
		}
		Engine::renderer.submit(Drawable(0, t.position, d.drawingPrio, d.scale, d.color, d.form, t.rotaVec, RenderSpace::WorldSpace, texRef.makeSmall()));
	}
	else {
		Engine::renderer.submit(Drawable(0, t.position, d.drawingPrio, d.scale, d.color, d.form, t.rotaVec, RenderSpace::WorldSpace));
	}
}
