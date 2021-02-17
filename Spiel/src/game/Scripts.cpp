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
	TextureHandle texHandle;
	Vec2 min{ 0,0 };
	Vec2 max{ 1,1 };
	if (game.world.hasComp<TextureSection>(entity)) {
		const auto& texSection = game.world.getComp<TextureSection>(entity);
		texHandle = texSection.handle;
		min = texSection.min;
		max = texSection.max;
	}
	game.renderer.drawSprite(Sprite{
		.color = d.color,
		.position = Vec3{t.position, -1.0f + f32(LAYER_WORLD_MIDGROUND) / f32(LAYER_MAX)},
		.rotationVec = t.rotaVec,
		.scale = d.scale,
		.texHandle = texHandle,
		.texMin = min,
		.texMax = max,
		.cornerRounding = (d.form == Form::Rectangle ? 0.0f : std::min(d.scale.x, d.scale.y) * 0.5f),
		.drawMode = RenderSpace::Camera,
	});
}
