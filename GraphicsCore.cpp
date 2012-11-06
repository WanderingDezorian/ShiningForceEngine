#include <SDL/SDL.h>
#include "GraphicsCore.h"
#include "ResourceCore.h"

unsigned int TileMapping::MaxBufferSize = 0;

void Sprite::Update(){
	switch(UpdatePattern){
		case UPDATE_LINEAR:
			CurrentOffset++;
			if(CurrentOffset == OrientationBufferOffset + OrientationBufferSize)
				CurrentOffset = OrientationBufferOffset;
			break;
		case UPDATE_BACKANDFORTH_FORWARD:
			if(CurrentOffset == OrientationBufferOffset + OrientationBufferSize - 1){
				CurrentOffset--;
				UpdatePattern = UPDATE_BACKANDFORTH_BACKWARD;
			}
			else
				CurrentOffset++;
			break;
		case UPDATE_BACKANDFORTH_BACKWARD:
			if(CurrentOffset == OrientationBufferOffset){
				CurrentOffset++;
				UpdatePattern = UPDATE_BACKANDFORTH_FORWARD;
			}
			else
				CurrentOffset--;
			break;
		case UPDATE_SINGLEPASSANDFREEZE:
			if(CurrentOffset == OrientationBufferOffset + OrientationBufferSize - 1)
				UpdatePattern = UPDATE_PAUSED;
			else
				CurrentOffset++;
			break;
		case UPDATE_SINGLEPASSANDVANISH:
			if(CurrentOffset == OrientationBufferOffset + OrientationBufferSize - 1)
				UpdatePattern = UPDATE_INVISIBLE;
			else
				CurrentOffset++;
			break;
	}
}

GraphicsCore::GraphicsCore() : MainWindow(0){
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
	const TileMapping* iSpriteLayer = CurrentState.TileLayers + CurrentState.SpriteLayerDepth;
	if(iSpriteLayer > CurrentState.TileLayersEnd)
		iSpriteLayer = CurrentState.TileLayersEnd;
	for(const TileMapping* iLayer = CurrentState.TileLayers; iLayer < iSpriteLayer; iLayer++){
		Point LayerSubIndex = ((CurrentState.MasterCamera * iLayer->ScaleNumerator) / iLayer->ScaleDenominator).min(iLayer->MaxCamera);
		Point LayerTileIndex = LayerSubIndex / 24;
		LayerSubIndex -= LayerTileIndex * 24;
		int Y, yEnd;
		SDL_Rect TempDest;
		for(Y = LayerTileIndex.Y, yEnd = Y + 11; Y < yEnd; Y++){
			DestRect.y = 24 * Y -LayerSubIndex.Y;
			DestRect.x = -LayerSubIndex.X;
			const unsigned int *iTile, *iTileEnd;
			for(iTile = iLayer->TileValues + Y * iLayer->SizeX + LayerTileIndex.X, iTileEnd = iTile + 15; iTile < iTileEnd; iTile++){
				if(*iTile != TileMapping::NOT_A_TILE){
					TileRect.x = *iTile * 24;
					TempDest = DestRect;
					SDL_BlitSurface(TileBuffer.Surface,&TileRect,MainWindow,&TempDest);
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
				DestRect.x = iSprite->Position.X - CurrentState.MasterCamera.X; // TODO:  Should these just be ints to avoid nastiness?
				DestRect.y = iSprite->Position.Y - CurrentState.MasterCamera.Y;
				SDL_BlitSurface(SpriteBuffer.Surface,&TileRect,MainWindow,&DestRect);
			}
		}
	}
	// Draw tile layers above sprite layer
	for(const TileMapping* iLayer = iSpriteLayer; iLayer < CurrentState.TileLayersEnd; iLayer++){
		Point LayerSubIndex = ((CurrentState.MasterCamera * iLayer->ScaleNumerator) / iLayer->ScaleDenominator).min(iLayer->MaxCamera);
		Point LayerTileIndex = LayerSubIndex / 24;
		LayerSubIndex -= LayerTileIndex * 24;
		int Y, yEnd;
		SDL_Rect TempDest;
		for(Y = LayerTileIndex.Y, yEnd = Y + 11; Y < yEnd; Y++){
			DestRect.y = 24 * Y -LayerSubIndex.Y;
			DestRect.x = -LayerSubIndex.X;
			const unsigned int *iTile, *iTileEnd;
			for(iTile = iLayer->TileValues + Y * iLayer->SizeX + LayerTileIndex.X, iTileEnd = iTile + 15; iTile < iTileEnd; iTile++){
				if(*iTile != TileMapping::NOT_A_TILE){
					TileRect.x = *iTile * 24;
					TempDest = DestRect;
					SDL_BlitSurface(TileBuffer.Surface,&TileRect,MainWindow,&TempDest);
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

