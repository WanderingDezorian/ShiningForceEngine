#include <SDL/SDL.h>
#include "GraphicsCore.h"

GraphicsCore::GraphicsCore() : MainWindow(0), TileBuffer(0){
	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) != 0)
		return;
	MainWindow = SDL_SetVideoMode(320, 240, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);

	TileRect.h = 24;
	TileRect.w = 24;
	TileRect.x = 0;
	TileRect.y = 0;

	DestRect.h = 24;
	DestRect.w = 24;
	DestRect.x = 0;
	DestRect.y = 0;

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

bool GraphicsCore::PrepareNextFrame(const GraphicalData &CurrentState){
	// Draw tile layers below sprite layer
	std::vector<TileMapping>::const_iterator iSpriteLayer;
	if(CurrentState.SpriteLayerDepth > CurrentState.TileLayers.size())
		iSpriteLayer = CurrentState.TileLayers.end();
	else
		iSpriteLayer = CurrentState.TileLayers.begin() + CurrentState.SpriteLayerDepth;
	CameraStruct SpriteLayerPosition = {0};
	for(std::vector<TileMapping>::const_iterator iLayer = CurrentState.TileLayers.begin(); iLayer < iSpriteLayer; iLayer++){
		SpriteLayerPosition = iLayer->Camera;
		int Y, yEnd;
		SDL_Rect TempDest;
		const unsigned int* TileMapBase = &(iLayer->TileValues.at(0));
		for(Y = iLayer->Camera.TileY, yEnd = Y + 11; Y < yEnd; Y++){
			DestRect.y = 24 * Y -iLayer->Camera.SubY;
			DestRect.x = -iLayer->Camera.SubX;
			const unsigned int *iTile, *iTileEnd;
			for(iTile = TileMapBase + Y * iLayer->SizeX + iLayer->Camera.TileX, iTileEnd = iTile + 15; iTile < iTileEnd; iTile++){
				if(*iTile != TileMapping::NOT_A_TILE){
					TileRect.x = *iTile * 24;
					TempDest = DestRect;
					SDL_BlitSurface(TileBuffer,&TileRect,MainWindow,&TempDest);
				}
				DestRect.x += 24;
			}
		}
	}
	// Draw sprite layer
	if(!CurrentState.AllSprites.empty()){
		const Sprite *iSprite, *iSpriteEnd;
		for(iSprite = &(CurrentState.AllSprites.at(0)), iSpriteEnd = iSprite + CurrentState.AllSprites.size(); iSprite < iSpriteEnd; iSprite++){
			if(iSprite->UpdatePattern != iSprite->UPDATE_INVISIBLE){
				TileRect.x = (iSprite->RootBufferOffset + iSprite->OrientationBufferOffset + iSprite->CurrentOffset) * 24;
				DestRect.x = (iSprite->Position.TileX - SpriteLayerPosition.TileX) * 24 + iSprite->Position.SubX - SpriteLayerPosition.SubX; // TODO:  Should these just be ints to avoid nastiness?
				DestRect.y = (iSprite->Position.TileY - SpriteLayerPosition.TileY) * 24 + iSprite->Position.SubY - SpriteLayerPosition.SubY;
				SDL_BlitSurface(TileBuffer,&TileRect,MainWindow,&DestRect);
			}
		}
	}
	// Draw tile layers above sprite layer
	for(std::vector<TileMapping>::const_iterator iLayer = iSpriteLayer; iLayer < CurrentState.TileLayers.end(); iLayer++){
		int Y, yEnd;
		SDL_Rect TempDest;
		const unsigned int* TileMapBase = &(iLayer->TileValues.at(0));
		for(Y = iLayer->Camera.TileY, yEnd = Y + 11; Y < yEnd; Y++){
			DestRect.y = 24 * Y -iLayer->Camera.SubY;
			DestRect.x = -iLayer->Camera.SubX;
			const unsigned int *iTile, *iTileEnd;
			for(iTile = TileMapBase + Y * iLayer->SizeX + iLayer->Camera.TileX, iTileEnd = iTile + 15; iTile < iTileEnd; iTile++){
				if(*iTile != TileMapping::NOT_A_TILE){
					TileRect.x = *iTile * 24;
					TempDest = DestRect;
					SDL_BlitSurface(TileBuffer,&TileRect,MainWindow,&TempDest);
				}
				DestRect.x += 24;
			}
		}
	}
	// Draw special layers
	for(std::vector<SpecialtyBuffer>::const_iterator iBuf = CurrentState.SpecialBuffers.begin(); iBuf != CurrentState.SpecialBuffers.end(); iBuf++)
		iBuf->Blit(MainWindow);
	return true;
}

bool GraphicsCore::AllocateTileBuffer(unsigned int Size){
	if(Size > TileBufferSize){
		if(TileBuffer)
			SDL_FreeSurface(TileBuffer);
		TileBuffer = SDL_CreateRGBSurface(
				(SDL_HWSURFACE & MainWindow->flags), 24 * Size, 24, MainWindow->format->BitsPerPixel,
				MainWindow->format->Rmask, MainWindow->format->Gmask, MainWindow->format->Bmask, MainWindow->format->Amask);
		if(TileBuffer)
			TileBufferSize = Size;
		else
			TileBufferSize = 0;
		return TileBuffer != 0;
	}
	return true;
}

bool GraphicsCore::LoadTileBuffer(const std::vector<std::string> &Filenames){
	if(!AllocateTileBuffer(Filenames.size()))
		return false;
	DestRect.x = 0;
	DestRect.y = 0;
	for(int i = 0; i < Filenames.size(); i++){
		SDL_Surface* Buffer = SDL_LoadBMP(Filenames[i].c_str());
		if(!Buffer)
			return false;
		TileRect.x = i*24;
		SDL_BlitSurface(Buffer,&DestRect,TileBuffer,&TileRect);
		SDL_FreeSurface(Buffer);
	}
	return true;
}
