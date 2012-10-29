#ifndef GRAPHICSCORE_H
#define GRAPHICSCORE_H

#include <SDL/SDL.h>
#include <vector>
#include <string>

struct CameraStruct{
	unsigned int TileX, TileY, SubX, SubY;
	void Reset(){ TileX = 0; TileY = 0; SubX = 0; SubY = 0; }
};

struct Sprite{
	CameraStruct Position;
	unsigned int RootBufferOffset;
	unsigned int OrientationBufferOffset;
	unsigned int OrientationBufferSize;
	unsigned int CurrentOffset;
	enum SpriteUpdatePatterns{
		UPDATE_LINEAR,
		UPDATE_BACKANDFORTH_FORWARD,
		UPDATE_BACKANDFORTH_BACKWARD,
		UPDATE_SINGLEPASSANDFREEZE,
		UPDATE_SINGLEPASSANDVANISH,
		UPDATE_PAUSED,
		UPDATE_INVISIBLE
	} UpdatePattern;
};

struct TileMapping{ // This is a layer
	const static unsigned int NOT_A_TILE = 0xFFFFFFFF;
	unsigned int SizeX;
	unsigned int SizeY;
	unsigned int* TileValues;
	unsigned int MainBufferOffset; //Offset to the first tile used in this map
	CameraStruct Camera;
	unsigned int TileValueBufferSize;
	TileMapping(): SizeX(0), SizeY(0), TileValues(0), MainBufferOffset(0), TileValueBufferSize(0) {};
	~TileMapping(){ if(TileValues) delete[] TileValues; }
	static unsigned int MaxBufferSize;
	bool Allocate(){ return Allocate(SizeX * SizeY); }
	bool Allocate(unsigned int Size){
		if(TileValueBufferSize >= Size)
			return true;
		if(TileValues)
			delete[] TileValues;
		TileValues = new unsigned int[Size];
		TileValueBufferSize = Size;
		return true;
	}
};

class SpecialtyBuffer{
	private:
		bool DrawMe;
		SDL_Rect PixDest;
		SDL_Surface* Buffer;
	public:
		SpecialtyBuffer(const char* Filename = 0) : DrawMe(false), Buffer(0){ PixDest.x = 0; PixDest.y = 0; if(Filename) Buffer = SDL_LoadBMP(Filename); }
		~SpecialtyBuffer(){ if(Buffer) SDL_FreeSurface(Buffer); }
		// TODO:  Add copy constructors that drop ownership of the buffer.
		bool Blit(SDL_Surface* MainScreen)const{ if(DrawMe) return SDL_BlitSurface(const_cast<SDL_Surface*>(Buffer),0,MainScreen,const_cast<SDL_Rect*>(&PixDest)) == 0; return false;}
		void SetEnable(bool EnableFlag){ if(Buffer) DrawMe = EnableFlag; }
		bool GetEnable() const{ return DrawMe; }
		void SetPixelOffset(short X, short Y){ PixDest.x = X; PixDest.y = Y; }
		bool Load(const char* Filename){ if(Buffer) SDL_FreeSurface(Buffer); if(Filename) Buffer = SDL_LoadBMP(Filename); return Buffer != 0; }
		SDL_Surface* AccessBuffer(){ return Buffer; }
};

struct GraphicalData{
	bool GraphicsRefreshRequired;
	bool GraphicsFlipRequired;
	TileMapping* TileLayers; // 0th layer drawn first
	const TileMapping* TileLayersEnd;
	unsigned int NumTileLayers;
	std::vector<Sprite> AllSprites;
	std::vector<SpecialtyBuffer> SpecialBuffers;
	unsigned int SpriteLayerDepth; // Depth i means sprite layer is drawn before layer i.

	GraphicalData() : TileLayers(0), NumTileLayers(0), TileLayersEnd(0), SpriteLayerDepth(0), GraphicsRefreshRequired(0), GraphicsFlipRequired(0) {};
	~GraphicalData(){ if(TileLayers) delete[] TileLayers; }
	void AllocateTileLayers(unsigned int NewNumLayers){
		if(TileLayers) delete[] TileLayers;
		if(NewNumLayers)
			TileLayers = new TileMapping[NewNumLayers];
		else
			TileLayers = 0;
		NumTileLayers = NewNumLayers;
		TileLayersEnd = TileLayers + NumTileLayers;
	}
	void Reset(){
		GraphicsRefreshRequired = true;
		GraphicsFlipRequired = false;
		for(TileMapping* iLayer = TileLayers; iLayer < TileLayersEnd; iLayer++){
			iLayer->Camera.Reset();
			iLayer->SizeX = 0;
		}
		AllSprites.clear();
		SpecialBuffers.clear();
	}
};

class GraphicsCore{
	private:
		SDL_Surface* MainWindow;
		SDL_Surface* TileBuffer; // TODO:  Should this be moved to Graphics data?
		unsigned int TileBufferSize;
		SDL_Rect TileRect;
		SDL_Rect DestRect;
		//SDL_Renderer* Renderer;

	public:
		GraphicsCore();
		~GraphicsCore();

		bool AllocateTileBuffer(unsigned int Size);
		bool LoadTileBuffer(const std::vector<std::string> &Filenames);
		bool LoadTileBuffer(unsigned int DestSlot, SDL_Surface* LoadFrom, Sint16 X, Sint16 Y);
		bool FlipBuffer();
		bool PrepareNextFrame(const GraphicalData &CurrentState);
		SDL_Surface* GetMainWindow(){ return MainWindow; }
};


#endif // GRAPHICSCORE_H
