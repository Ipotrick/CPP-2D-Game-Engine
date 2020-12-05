#include "HealthScript.hpp"
#include "LayerConstants.hpp"
#include "../engine/ui/UICreate.hpp"

void createHealthUI(EntityHandle ent)
{
	auto barUpdate =
		[&, ent](UIElement* e) {
		if (e->isEnabled()) {
			UIBar* b = (UIBar*)e;
			Health& h = Game::world.getComp<Health>(ent);
			b->setFill((float)h.curHealth / (float)h.maxHealth);
		}
	};
	auto frameUpdateFn =
		[&, ent](UIElement* e) {
		if (e->isEnabled()) {
			UIFrame* f = (UIFrame*)e;
			Vec2 pos = EngineCore::renderer.convertCoordinate<RenderSpace::WorldSpace, RenderSpace::PixelSpace>(Game::world.getComp<Transform>(ent).position);
			float scale = EngineCore::renderer.getCamera().zoom * 3.5f;
			f->anchor.setAbsPosition(pos + Vec2(0.0f, 60.0f) * scale);
			f->setScale(scale);
		}
	};
	auto frameDestroyIfFn =
		[&, ent](UIElement* e) -> bool {
		return !(Game::world.isHandleValid(ent) && Game::world.hasComp<Health>(ent)) || !Game::world.getComp<Health>(ent).bUISpawned;
	};
	auto frameEnableIfFn =
		[&, ent](UIElement* e) -> bool {
		Health& h = Game::world.getComp<Health>(ent);
		return h.curHealth != h.maxHealth;
	};

	ui::frame({
		.anchor = UIAnchor({
			.xmode = UIAnchor::X::DirectPosition,
			.ymode = UIAnchor::Y::DirectPosition }),
		.size = {110,50},
		.fn_update = frameUpdateFn,
		.fn_enableIf = frameEnableIfFn,
		.focusable = false,
		.padding = 5,
		.fillColor = {0,0,0,0.5},
		.layer = LAYER_FIRST_UI,
		.fn_destroyIf = frameDestroyIfFn,
		.borders = {0,0},
		.drawMode = RenderSpace::PixelSpace 
	},
	{
		ui::pair({.size = {110,50} },
		{
			ui::text({
				.anchor = {{
					.xmode = UIAnchor::X::LeftRelativeDist,
					.x = 0.5,
					.ymode = UIAnchor::Y::TopAbsoluteDist,
					.y = 0
				}},
				.size = {100,20},
				.textAnchor = {{
					.xmode = UIAnchor::X::LeftRelativeDist,
					.x = 0.5,
					.ymode = UIAnchor::Y::TopRelativeDist,
					.y = 0.5 
				}},
				.text = "Health:",
				.fontTexture = EngineCore::renderer.makeSmallTexRef(TextureInfo("ConsolasAtlas2.png")),
				.fontSize = {17.0f / 2.0f, 17.0f}
			}),
			ui::bar({
				.anchor = {{
					.xmode = UIAnchor::X::LeftRelativeDist,
					.x = 0.5 
				}},
				.size = {100,20},
				.fn_update = barUpdate 
			})
		})
	});
}

void healthScript(EntityHandle me, Health& data, float deltaTime)
{
	World& world = Game::world;
	bool gotHitByBullet{ false };
	for (auto collision : Game::collisionSystem.collisions_view(me)) {
		if (world.hasComp<Bullet>(collision.indexB)) {
			gotHitByBullet = true;
		}
	}

	if (data.curHealth <= 0) {
		world.destroy(me);
	}
	else if (data.curHealth != data.maxHealth && !data.bUISpawned) {
		createHealthUI(me);
		data.bUISpawned = true;
	}
}