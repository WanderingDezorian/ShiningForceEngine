#ifndef RESOURCE_CORE_H
#define RESOURCE_CORE_H

#include <SDL/SDL.h>
#include "GraphicsCore.h"

bool InitializeResources(const char* MapFilename, GraphicsCore& Core, GraphicalData &Data); // Update later to support zip-file resource set

SDL_Surface* LoadPng(const char* Filename);

// TODO:  Add song loader, wav loader

// TODO:  Add layer loader- Data is base64 encoded, then gzipped or zlibbed.  Data is little-endian uint32s, each with tile gids.
//  Within a tileset, gids are read row-major format (C++ style, not matlab).

int LoadMap(const char* Filename, GraphicsCore &Core, TileMapping* TileLayers, unsigned int NumLayers); // Returns number of layers provided
bool GetMapInfo(const char* Filename, unsigned int &NumLayers, unsigned int &MaxSizeXinTiles, unsigned int &MaxSizeYinTiles,unsigned int &NumUniqueTiles);

#endif
