//#define MANDELBROT
//#define BALLS2
#define GUITEST

#ifdef BALLS2

#include "game/Game.hpp"

int main() 
{
	Game game;
	game.run();
}
#endif

#ifdef MANDELBROT

#include "mandelbrot/MandelbrotViewer.hpp"

int main()
{
	MandelbrotViewer mv;
	mv.initialize("Mandelbrot Viewer", 800ull, 800ull);
	mv.run();
}
#endif

#ifdef GUITEST

#include "guitest/GUITest.hpp"

int main()
{
	GUIApp app;
	app.run();
}

#endif
