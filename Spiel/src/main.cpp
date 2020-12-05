#define BALLS2

#ifdef BALLS2

#include "game/Game.hpp"

int main() 
{
	Game game;
	game.run();
}

#else
#include "mandelbrot/MandelbrotViewer.hpp"

int main()
{
	MandelbrotViewer mv;
	mv.initialize("Mandelbrot Viewer", 800ull, 800ull);
	mv.run();
}

#endif