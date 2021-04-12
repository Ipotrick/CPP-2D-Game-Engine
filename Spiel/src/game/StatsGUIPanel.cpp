#include "StatsGUIPanel.hpp"

GUIStatsPanel::GUIStatsPanel(Game* game):
	game{game}
{
	using namespace gui;

	Manager& gui = game->gui;
	rootHandle = gui.build(Root{
		.onUpdate = [&](Root& self,u32 id) { update(); },
		.sizing = Sizing{}.absX(200).absY(300),
		.placing = Placing{}.absDistLeft(20).absDistTop(40),
		.child = gui.build(Box{
			.bFillSpace = true,
			.bDragable = true,
			.bScreenTexture = true,
			.texture = TextureSection{game->renderer.tex.getHandle("blur")},
			.color = Vec4{0.3,0.3,0.3,1},
			.child = gui.build(Column{
				.children = {
					gui.build(Box{
						.bFillSpace = true,
						.color = Vec4{0,0,0,0},
						.xalign = XAlign::Center,
						.child = gui.build(StaticText{.value = "Statistics:", .fontSize = 16})
					}),
					gui.build(Text{.value = &entityCountStr}),
					gui.build(Text{.value = &fpsStr}),
					gui.build(SliderF64{
						.value = &impResIterSliderValue, 
						.min = 1.0f, 
						.max = 50.0f, 
						.bThin = false,
						.colorBar = Vec4{0,0,0,0.2},
						.colorSlider = Vec4{1,1,1,0.4},
						.child = gui.build(Text{
							.onUpdate = 
							[&](Text& self, u32 id) {
								self.value = std::string("PhysicsIterations ") + std::to_string(cast<u32>(impResIterSliderValue));
							}
						})
					})
				}
			})
		})
	});
}

GUIStatsPanel::~GUIStatsPanel()
{
	if (game->gui.isHandleValid(rootHandle)) {
		game->gui.destroy(rootHandle);
	}
}

void GUIStatsPanel::update()
{
	entityCountStr =	std::string("entitycount: ") + std::to_string(game->world.size());
	fpsStr =			std::string("fps:         ") + std::to_string(std::ceilf(1.0f / game->getDeltaTime(20)));
	game->physicsSystem2.settings.impulseResolutionIterations = cast<u32>(impResIterSliderValue);
}
