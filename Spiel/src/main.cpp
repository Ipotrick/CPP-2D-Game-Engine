//#define MANDELBROT
//#define BALLS2
//#define GUITEST
#define ANTS

#ifdef BALLS2

#include "game/Game.hpp"

int main() 
{
	globalInitialize();
	Game game;
	game.run();
}
#endif

#ifdef MANDELBROT

#include "mandelbrot/MandelbrotViewer.hpp"

int main()
{
	globalInitialize();
	MandelbrotViewer mv;
	mv.initialize("Mandelbrot Viewer", 800ull, 800ull);
	mv.run();
}
#endif

#ifdef GUITEST

#include "guitest/GUITest.hpp"

int main()
{
	globalInitialize();
	auto* app = new GUIApp();
	app->run();
	delete app;
}

#endif

#ifdef ANTS

#include "Ants/Ants.hpp"

int main()
{
	globalInitialize();
	AntsApp app;
	app.run();

}

#endif
