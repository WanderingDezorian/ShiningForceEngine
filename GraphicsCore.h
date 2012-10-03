#ifndef GRAPHICSCORE_H
#define GRAPHICSCORE_H

#include <SDL/SDL.h>
#include "GameState.h"

class GraphicsCore{
	private:
		SDL_Surface* MainWindow;
		//SDL_Renderer* Renderer;

	public:
		GraphicsCore();
		~GraphicsCore();

		bool FlipBuffer();
		bool PrepareNextFrame(const GameState &CurrentState);
		SDL_Surface* GetMainWindow(){ return MainWindow; }
};

#endif // GRAPHICSCORE_H
