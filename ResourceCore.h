#ifndef RESOURCE_CORE_H
#define RESOURCE_CORE_H

#include <SDL/SDL.h>
#include "GraphicsCore.h"

bool InitializeResources(const char* MapFilename, GraphicsCore& Core, GraphicalData &Data); // Update later to support zip-file resource set

SDL_Surface* LoadPng(const char* Filename);

// TODO:  Add song loader, wav loader

int LoadMap(const char* Filename, GraphicsCore &Core, TileMapping* TileLayers, unsigned int NumLayers); // Returns number of layers provided
bool GetMapInfo(const char* Filename, unsigned int &NumLayers, unsigned int &MaxSizeXinTiles, unsigned int &MaxSizeYinTiles,unsigned int &NumUniqueTiles);

#endif
