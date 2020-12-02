#include "HealthScript.hpp"
#include "LayerConstants.hpp"

void createHealthUI(EntityHandle ent)
{
	UIBar bar;
	bar.setSize({ 100.0f, 20.0f });
	bar.anchor.setCenterHorizontal();
	bar.setUpdateFn(
		[&, ent](UIElement* e) {
			if (e->isEnabled()) {
				UIBar* b = (UIBar*)e;
				Health& h = Game::world.getComp<Health>(ent);
				b->setFill((float)h.curHealth / (float)h.maxHealth);
			}
		}
	);

	UIText t("Health:", EngineCore::renderer.makeTexRef(TextureInfo("ConsolasLowRes.png")).makeSmall());
	t.setSize({ 100.0f, 20.0f });
	t.fontSize = { 17.0f / 2.0f, 17.0f };
	t.anchor.setCenterHorizontal();
	t.anchor.setTopAbsolute(0.0f);
	t.textAnchor.setCenterVertical();
	t.textAnchor.setCenterHorizontal();

	UIPair p;
	p.setSize({ 110, 50 });
	p.setFirst(EngineCore::ui.createAndGet(t));
	p.setSecond(EngineCore::ui.createAndGet(bar));

	UIFrame frame;
	frame.layer = LAYER_FIRST_UI;
	frame.fillColor = { 0,0,0, 0.5 };
	frame.setFocusable(false);
	frame.setBorders(0);
	frame.setSize({ 110, 50 });
	frame.setPadding({ 5,5 });
	frame.setDrawMode(RenderSpace::PixelSpace);
	frame.addChild(EngineCore::ui.createAndGet(p));
	frame.setUpdateFn(
		[&, ent](UIElement* e) {
			if (e->isEnabled()) {
				UIFrame* f = (UIFrame*)e;
				Vec2 pos = EngineCore::renderer.convertCoordinate<RenderSpace::WorldSpace, RenderSpace::PixelSpace>(Game::world.getComp<Transform>(ent).position);
				float scale = EngineCore::renderer.getCamera().zoom * 3.5f;
				f->anchor.setAbsPosition(pos + Vec2(0.0f, 60.0f) * scale);
				f->setScale(scale);
			}
		}
	);
	frame.setDestroyIfFn(
		[&, ent](UIElement* e) -> bool {
			return !(Game::world.isHandleValid(ent) && Game::world.hasComp<Health>(ent)) || !Game::world.getComp<Health>(ent).bUISpawned;
		}
	);
	frame.setEnableIfFn(
		[&, ent](UIElement* e) -> bool {
			Health& h = Game::world.getComp<Health>(ent);
			return h.curHealth != h.maxHealth;
		}
	);

	EngineCore::ui.createFrame(frame);
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