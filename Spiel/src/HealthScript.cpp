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
				Health& h = Engine::world.getComp<Health>(ent);
				b->setFill((float)h.curHealth / (float)h.maxHealth);
			}
		}
	);

	UIText t("Health:", Engine::renderer.makeTexRef(TextureInfo("ConsolasLowRes.png")).makeSmall());
	t.setSize({ 100.0f, 20.0f });
	t.fontSize = { 17.0f / 2.0f, 17.0f };
	t.anchor.setCenterHorizontal();
	t.anchor.setTopAbsolute(0.0f);
	t.textAnchor.setCenterVertical();
	t.textAnchor.setCenterHorizontal();

	UIPair p;
	p.setSize({ 110, 50 });
	p.setFirst(Engine::ui.createAndGet(t));
	p.setSecond(Engine::ui.createAndGet(bar));

	UIFrame frame;
	frame.layer = LAYER_FIRST_UI;
	frame.fillColor = { 0,0,0, 0.5 };
	frame.setFocusable(false);
	frame.setBorders(0);
	frame.setSize({ 110, 50 });
	frame.setPadding({ 5,5 });
	frame.setDrawMode(RenderSpace::PixelSpace);
	frame.addChild(Engine::ui.createAndGet(p));
	frame.setUpdateFn(
		[&, ent](UIElement* e) {
			if (e->isEnabled()) {
				UIFrame* f = (UIFrame*)e;
				Vec2 pos = Engine::renderer.convertCoordinate<RenderSpace::WorldSpace, RenderSpace::PixelSpace>(Engine::world.getComp<Transform>(ent).position);
				float scale = Engine::renderer.getCamera().zoom * 3.5f;
				f->anchor.setAbsPosition(pos + Vec2(0.0f, 60.0f) * scale);
				f->setScale(scale);
			}
		}
	);
	frame.setDestroyIfFn(
		[&, ent](UIElement* e) -> bool {
			return !(Engine::world.isHandleValid(ent) && Engine::world.hasComp<Health>(ent)) || !Engine::world.getComp<Health>(ent).bUISpawned;
		}
	);
	frame.setEnableIfFn(
		[&, ent](UIElement* e) -> bool {
			Health& h = Engine::world.getComp<Health>(ent);
			return h.curHealth != h.maxHealth;
		}
	);

	Engine::ui.createFrame(frame);
}

void healthScript(EntityHandle me, Health& data, float deltaTime)
{
	World& world = Engine::world;
	bool gotHitByBullet{ false };
	for (auto collision : Game::collisionSystem.collisions_view(me)) {
		if (world.hasComp<Bullet>(collision.indexB)) {
			gotHitByBullet = true;
		}
	}

	if (data.curHealth <= 0) {
		world.destroy(me);
	}

	if (!data.bUISpawned) {
		createHealthUI(me);
		data.bUISpawned = true;
	}
}