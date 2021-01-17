#include "drawScript.hpp"
#include "LayerConstants.hpp"

void drawScript(EntityHandle entity, const Transform& t, const Draw& d)
{
	int layer = d.bParticleLayer ? LAYER_WORLD_PARTICLE : LAYER_WORLD_MIDGROUND;
	if (Game::world.hasComp<BigTextureRef>(entity)) {
		BigTextureRef& texRef = Game::world.getComp<BigTextureRef>(entity);
		if (!texRef.good()) {
			// if a TexRef component was created without the renderer, it will be replaced here:
			EngineCore::renderer.validateTextureRef(texRef);
		}
		EngineCore::renderer.submit(
			Sprite{
				.color = d.color,
				.position = Vec3{t.position.x, t.position.y, clamp(d.drawingPrio, 0.0f, 1.0f) },
				.rotationVec = t.rotaVec,
				.scale = d.scale,
				.texRef=texRef.makeSmall(),
				.form = d.form,
				.drawMode = RenderSpace::WorldSpace
			},
			layer
		);
	}
	else {
		EngineCore::renderer.submit(
			Sprite{
				.color = d.color,
				.position = Vec3{t.position.x, t.position.y, clamp(d.drawingPrio, 0.0f, 1.0f) },
				.rotationVec = t.rotaVec,
				.scale = d.scale,
				.form = d.form,
				.drawMode=RenderSpace::WorldSpace
			},
			layer
		);
	}
}
