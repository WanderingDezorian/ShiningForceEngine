#ifndef RESOURCE_CORE_H
#define RESOURCE_CORE_H

#include "MiniZip/unzip.h"
#include <SDL/SDL.h>
#include <map>
#include "GameState.h"

bool InitializeResources(GraphicsCore& Core, GameState &Data); // Update later to support zip-file resource set
bool LoadLevel(const std::string &LevelName, GraphicsCore& GCore, GameState &Data); // Update later to support zip-file resource set
bool DefineGlobalZipfile(const char* ZipfileName);

SDL_Surface* LoadPng(const char* Filename);

// TODO:  Add song loader, wav loader

int LoadMap(const char* Filename, GraphicsCore& GCore, GameState &Data, const char* EntryPointName, Point &EntryPoint); // Returns number of layers provided
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

class ZipfileInterface{ // TODO:  Make singleton
	unzFile zFile;
	bool ChildFileOpen;
	bool OpenChild();
	void CloseChild();
public:
	ZipfileInterface() : zFile(0), ChildFileOpen(false) {}
	~ZipfileInterface(){ CloseFile(); }
	bool OpenFile(const char* Filename);
	bool IsOpen()const;
	void CloseFile();

	bool ContainsFile(const char* Filename)const;
	unsigned int Filesize()const;
	unsigned int Filesize(const char* Filename)const;
	bool IsStoredUncompressed()const;
	bool IsStoredUncompressed(const char* Filename)const;
	bool Uncompress(unsigned char* Buffer, unsigned int BufferLength);
	bool Uncompress(const char* Filename,unsigned char* Buffer, unsigned int BufferLength);
	int UncompressInexact(unsigned char* Buffer, unsigned int BufferLength);
	int UncompressInexact(const char* Filename,unsigned char* Buffer, unsigned int BufferLength);
};

#endif
