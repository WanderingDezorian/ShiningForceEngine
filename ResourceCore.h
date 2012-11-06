#ifndef RESOURCE_CORE_H
#define RESOURCE_CORE_H

#include <SDL/SDL.h>
#include "GraphicsCore.h"

bool InitializeResources(const char* MapFilename, GraphicsCore& Core, GraphicalData &Data); // Update later to support zip-file resource set

SDL_Surface* LoadPng(const char* Filename);

// TODO:  Add song loader, wav loader

int LoadMap(const char* Filename, GraphicsCore &Core, TileMapping* TileLayers, unsigned int NumLayers); // Returns number of layers provided
bool GetMapInfo(const char* Filename, unsigned int &NumLayers, unsigned int &MaxSizeXinTiles, unsigned int &MaxSizeYinTiles,unsigned int &NumUniqueTiles);

class FileGuard{
	FILE *myPointer;
public:
	FileGuard(FILE *ToGuard = 0) : myPointer(ToGuard){}
	~FileGuard(){ if(myPointer) fclose(myPointer); }
	void GuardFile(FILE* ToGuard){ myPointer = ToGuard; }
	void StopGuarding(){ myPointer = 0; }
};

class SurfaceGuard{
	SDL_Surface* myPointer;
public:
	SurfaceGuard(SDL_Surface *ToGuard = 0) : myPointer(ToGuard){}
	~SurfaceGuard(){ if(myPointer) SDL_FreeSurface(myPointer); }
	void GuardSurface(SDL_Surface* ToGuard){ myPointer = ToGuard; }
	void StopGuarding(){ myPointer = 0; }
	void Delete(){ if(myPointer) SDL_FreeSurface(myPointer); myPointer = 0; }
};

#endif
