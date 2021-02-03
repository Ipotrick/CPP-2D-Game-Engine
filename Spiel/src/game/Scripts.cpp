#include "Scripts.hpp"

void ageScript(Game& game, EntityHandle id, Age& data, float deltaTime)
{
	data.curAge += deltaTime;

	if (data.curAge > data.maxAge) {
		game.world.destroy(id);
	}
}

void bulletScript(Game& game, EntityHandle me, Bullet& data, float deltaTime)
{
	auto& draw = game.world.getComp<Draw>(me);
	Age& age = game.world.getComp<Age>(me);
	draw.color.w = 1.0f - age.curAge / age.maxAge;

	bool foundHit{ false };
	for (CollisionInfo const& collision : game.collisionSystem.collisions_view(me.index)) {
		if (game.world.hasComps<Collider, PhysicsBody>(collision.indexB) && game.world.hasntComp<Player>(collision.indexB)) {
			foundHit = true;
		}
		if (game.world.hasComp<Health>(collision.indexB)) {
			game.world.getComp<Health>(collision.indexB).curHealth -= data.damage;
		}
	}
	if (foundHit == true) {
		// data.hitPoints -= 1;
		data.hitPoints = 0;
	}
	if (data.hitPoints <= 0) {
		game.world.despawn(me);
		game.world.destroy(me);
	}
}

#include "LayerConstants.hpp"
void drawScript(Game& game, EntityHandle entity, const Transform& t, const Draw& d)
{
	int layer = d.bParticleLayer ? LAYER_WORLD_PARTICLE : LAYER_WORLD_MIDGROUND;
	if (game.world.hasComp<BigTextureRef>(entity)) {
		BigTextureRef& texRef = game.world.getComp<BigTextureRef>(entity);
		if (!texRef.good()) {
			// if a TexRef component was created without the renderer, it will be replaced here:
			game.renderer.validateTextureRef(texRef);
		}
		game.renderer.submit(
			Sprite{
				.color = d.color,
				.position = Vec3{t.position.x, t.position.y, clamp(d.drawingPrio, 0.0f, 1.0f) },
				.rotationVec = t.rotaVec,
				.scale = d.scale,
				.textureId = texRef.getId(),
				.minTex = texRef.makeSmall().minPos,
				.maxTex = texRef.makeSmall().maxPos,
				.form = d.form,
				.drawMode = RenderSpace::WorldSpace
			},
			layer
		);
	}
	else {
		game.renderer.submit(
			Sprite{
				.color = d.color,
				.position = Vec3{t.position.x, t.position.y, clamp(d.drawingPrio, 0.0f, 1.0f) },
				.rotationVec = t.rotaVec,
				.scale = d.scale,
				.form = d.form,
				.drawMode = RenderSpace::WorldSpace
			},
			layer
		);
	}
}
