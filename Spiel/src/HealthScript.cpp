#include "HealthScript.hpp"

void HealthScript::script(EntityHandle me, Health& data, float deltaTime)
{
	auto createUI = 
		[&](EntityHandle id)
	{
		UIBar bar;
		bar.setSize({ 100.0f, 20.0f });
		bar.anchor.setCenterHorizontal();
		bar.setUpdateFn(
			[&, id](UIElement* e) {
				if (e->isEnabled()) {
					UIBar* b = (UIBar*)e;
					Health& h = world.getComp<Health>(id);
					b->setFill((float)h.curHealth / (float)h.maxHealth);
				}
			}
		);

		UIText t("Health:", engine.renderer.makeTexRef(TextureInfo("ConsolasLowRes.png")).makeSmall());
		t.setSize({ 100.0f, 20.0f });
		t.fontSize = { 17.0f / 2.0f, 17.0f };
		t.anchor.setCenterHorizontal();
		t.anchor.setTopAbsolute(0.0f);
		t.textAnchor.setCenterVertical();
		t.textAnchor.setCenterHorizontal();

		UIPair p;
		p.setSize({ 110, 50 });
		p.setFirst(engine.ui.createAndGet(t));
		p.setSecond(engine.ui.createAndGet(bar));

		UIFrame frame;
		frame.fillColor = { 0,0,0, 0.5 };
		frame.setFocusable(false);
		frame.setBorders(0);
		frame.setSize({ 110, 50 });
		frame.setPadding({ 5,5 });
		frame.setDrawMode(RenderSpace::PixelSpace);
		frame.addChild(engine.ui.createAndGet(p));
		frame.setUpdateFn(
			[&, id](UIElement* e) {
				UIFrame* f = (UIFrame*)e;
				Vec2 windowPos = engine.getWorldToWindow(world.getComp<Transform>(id).position);
				Vec2 pixelPos = engine.getWindowToPixel(windowPos);
				float scale = engine.camera.zoom * 3.5f;
				f->anchor.setAbsPosition(pixelPos + Vec2(0.0f, 60.0f) * scale);
				f->setScale(scale);
			}
		);
		frame.setDestroyIfFn(
			[&, id](UIElement* e) -> bool {
				return !(world.isHandleValid(id) && world.hasComp<Health>(id)) || !world.getComp<Health>(id).bUISpawned;
			}
		);
		frame.setEnableIfFn(
			[&, id](UIElement* e) -> bool {
				Health& h = world.getComp<Health>(id);
				return h.curHealth != h.maxHealth;
			}
		);

		engine.ui.createFrame(frame);
	};

	World& world = engine.world;
	bool gotHitByBullet{ false };
	for (auto collision : engine.collisionSystem.collisions_view(me)) {
		if (world.hasComp<Bullet>(collision.indexB)) {
			gotHitByBullet = true;
		}
	}

	if (data.curHealth <= 0) {
		world.destroy(me);
	}

	if (!data.bUISpawned) {
		createUI(me);
		data.bUISpawned = true;
	}
}