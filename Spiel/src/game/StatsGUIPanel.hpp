#pragma once

#include "Game.hpp"

class GUIStatsPanel {
public:
	GUIStatsPanel(Game* game);
	~GUIStatsPanel();
private:
	gui::Manager::RootHandle rootHandle;
	Game* game;

	void update();

	f64 impResIterSliderValue{ 5.0f };
	std::string entityCountStr;
	std::string fpsStr;
};