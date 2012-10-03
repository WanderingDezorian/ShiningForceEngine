#include <SDL/SDL.h>
#include "GraphicsCore.h"

GraphicsCore::GraphicsCore() : MainWindow(0){
	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) != 0)
		return;
	MainWindow = SDL_SetVideoMode(320, 240, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);

	// THIS IS SDL 2.0 code
/*	MainWindow = SDL_CreateWindow("SDL_RenderClear",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		512, 512,
		SDL_WINDOW_SHOWN); //TODO:  Make SDL_WINDOW_FULLSCREEN eventually.
	if(MainWindow)
		Renderer = SDL_CreateRenderer(MainWindow, -1, SDL_RENDERER_ACCELERATED); */
}
GraphicsCore::~GraphicsCore(){
//	if(Renderer)
//		SDL_DestroyRenderer(Renderer);
//	if(MainWindow)
//		SDL_DestroyWindow(MainWindow);
	SDL_Quit(); // Also releases main window
}

bool GraphicsCore::FlipBuffer(){
	SDL_Flip(MainWindow);
	return true;
//	RenderPresent(Renderer);
}

bool GraphicsCore::PrepareNextFrame(const GameState &CurrentState){
	return true;
}
