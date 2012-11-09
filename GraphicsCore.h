#ifndef GRAPHICSCORE_H
#define GRAPHICSCORE_H

#include <SDL/SDL.h>
#include <vector>
#include <string>

struct Point{
	unsigned int X,Y; // TODO:  Does TileX/Y serve any purpose?
	Point() : X(0), Y(0) {}
	Point(const unsigned int &Val) : X(Val), Y(Val) {}
	Point(const unsigned int &InitialX, const unsigned int &InitialY) : X(InitialX), Y(InitialY) {}
	Point& operator=(const Point &Val){ X = Val.X; Y = Val.Y; return *this; }
	Point& operator=(const unsigned int &Val){ X = Val; Y = Val; return *this; }

	bool operator==(const Point &Val){ return (X == Val.X) && (Y == Val.Y); }
	Point operator+(const Point &Val)const{ return Point(X+Val.X,Y+Val.Y); }
	Point operator-(const Point &Val)const{ return Point(X-Val.X,Y-Val.Y); }
	Point operator*(const Point &Val)const{ return Point(X*Val.X,Y*Val.Y); }
	Point operator/(const Point &Val)const{ return Point(X/Val.X,Y/Val.Y); }
	Point operator+(const unsigned int &Val)const{ return Point(X+Val,Y+Val); }
	Point operator-(const unsigned int &Val)const{ return Point(X-Val,Y-Val); }
	Point operator*(const unsigned int &Val)const{ return Point(X*Val,Y*Val); }
	Point operator/(const unsigned int &Val)const{ return Point(X/Val,Y/Val); }

	Point& operator+=(const Point &Val){ X += Val.X; Y += Val.Y; return *this; }
	Point& operator-=(const Point &Val){ X -= Val.X; Y -= Val.Y; return *this; }
	Point& operator*=(const Point &Val){ X *= Val.X; Y *= Val.Y; return *this; }
	Point& operator/=(const Point &Val){ X /= Val.X; Y /= Val.Y; return *this; }
	Point& operator+=(const unsigned int &Val){ X += Val; Y += Val; return *this; }
	Point& operator-=(const unsigned int &Val){ X -= Val; Y -= Val; return *this; }
	Point& operator*=(const unsigned int &Val){ X *= Val; Y *= Val; return *this; }
	Point& operator/=(const unsigned int &Val){ X /= Val; Y /= Val; return *this; }

	Point min(const Point &Val)const{ return Point( (X < Val.X ? X : Val.X), (Y < Val.Y ? Y : Val.Y) ); }
	Point min(const unsigned int &Val)const{ return Point( (X < Val ? X : Val), (Y < Val ? Y : Val) ); }
	Point& minEq(const Point &Val){ if(Val.X < X) X = Val.X; if(Val.Y < Y) Y = Val.Y; return *this; }
	Point& minEq(const unsigned int &Val){ if(Val < X) X = Val; if(Val < Y) Y = Val; return *this; }

	Point max(const Point &Val)const{ return Point( (X > Val.X ? X : Val.X), (Y > Val.Y ? Y : Val.Y) ); }
	Point max(const unsigned int &Val)const{ return Point( (X > Val ? X : Val), (Y > Val ? Y : Val) ); }
	Point& maxEq(const Point &Val){ if(Val.X > X) X = Val.X; if(Val.Y > Y) Y = Val.Y; return *this; }
	Point& maxEq(const unsigned int &Val){ if(Val > X) X = Val; if(Val > Y) Y = Val; return *this; }

	void MoveTowards(const Point &Val, const unsigned int Speed = 1){
		if(Val.X > X)
			X = (X + Speed > Val.X) ? Val.X : X + Speed;
		else if(Val.X < X)
			X = (X - Speed < Val.X) ? Val.X : X - Speed;
		if(Val.Y > Y)
			Y = (Y + Speed > Val.Y) ? Val.Y : Y + Speed;
		else if(Val.Y < Y)
			Y = (Y - Speed < Val.Y) ? Val.Y : Y - Speed;
	}
};

template<int BlockSize> struct BlockBuffer{
	SDL_Surface* Surface;
	unsigned int Size;
public:
	BlockBuffer() : Surface(0), Size(0) {};
	~BlockBuffer(){ if(Surface) SDL_FreeSurface(Surface); }
	bool Load(unsigned int DestSlot, SDL_Surface* LoadFrom, Sint16 X, Sint16 Y){
		SDL_Rect SourceRect;
		SourceRect.h = BlockSize;
		SourceRect.w = BlockSize;
		SourceRect.x = X;
		SourceRect.y = Y;
		SDL_Rect DestRect;
		DestRect.h = BlockSize;
		DestRect.w = BlockSize;
		DestRect.y = 0;
		DestRect.x = DestSlot*BlockSize;
		SDL_BlitSurface(LoadFrom,&SourceRect,Surface,&DestRect);
		return true;
	}
	bool Allocate(const SDL_Surface* MainWindow, unsigned int NewSize){
		if(NewSize > Size){
			if(Surface)
				SDL_FreeSurface(Surface);
			Surface = SDL_CreateRGBSurface(
					(SDL_HWSURFACE & MainWindow->flags), BlockSize * NewSize, BlockSize, MainWindow->format->BitsPerPixel,
					MainWindow->format->Rmask, MainWindow->format->Gmask, MainWindow->format->Bmask, MainWindow->format->Amask);
			if(Surface)
				Size = NewSize;
			else
				Size = 0;
			return Size != 0;
		}
		return true;
	}
};

struct Sprite{
	Point Position;
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
	void Update();
};

struct TileMapping{ // This is a layer
	const static unsigned int NOT_A_TILE = 0xFFFFFFFF;
	unsigned int SizeX;
	unsigned int SizeY;
	unsigned int* TileValues;
	unsigned int MainBufferOffset; //Offset to the first tile used in this map
	Point MaxCamera;
	Point ScaleNumerator;
	Point ScaleDenominator;
	bool WrapX, WrapY;
	unsigned int TileValueBufferSize;
	TileMapping(): SizeX(0), SizeY(0), TileValues(0), MainBufferOffset(0), TileValueBufferSize(0), WrapX(false), WrapY(false) {};
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
	const TileMapping* TileLayersBufferEnd;
	unsigned int NumTileLayers;
	std::vector<Sprite> AllSprites;
	std::vector<SpecialtyBuffer> SpecialBuffers;
	unsigned int SpriteLayerDepth; // Depth i means sprite layer is drawn before layer i.
	Point MasterCamera;
	Point DesiredMasterCamera;
	Point MasterMapSizeInTiles;

	GraphicalData() : AllSprites(), SpecialBuffers(), TileLayers(0), NumTileLayers(0), TileLayersEnd(0), SpriteLayerDepth(0), GraphicsRefreshRequired(0), GraphicsFlipRequired(0) {};
	~GraphicalData(){ if(TileLayers) delete[] TileLayers; }
	void AllocateTileLayers(unsigned int NewNumLayers){
		if(TileLayers) delete[] TileLayers;
		if(NewNumLayers)
			TileLayers = new TileMapping[NewNumLayers];
		else
			TileLayers = 0;
		NumTileLayers = NewNumLayers;
		TileLayersBufferEnd = TileLayers + NumTileLayers;
	}
	void Reset(){
		GraphicsRefreshRequired = true;
		GraphicsFlipRequired = false;
		for(TileMapping* iLayer = TileLayers; iLayer < TileLayersBufferEnd; iLayer++){
			iLayer->MaxCamera = 0xFFFFFFFF;
			iLayer->ScaleNumerator = 1;
			iLayer->ScaleDenominator = 1;
			iLayer->SizeX = 0;
		}
		AllSprites.clear();
		SpecialBuffers.clear();
		MasterCamera = 0;
		TileLayersEnd = TileLayers;
	}
};

class GraphicsCore{
	private:
		SDL_Surface* MainWindow;
		BlockBuffer<24> TileBuffer;
		BlockBuffer<24> SpriteBuffer;
		SDL_Rect TileRect;
		SDL_Rect DestRect;
		//SDL_Renderer* Renderer;

	public:
		GraphicsCore();
		~GraphicsCore();

		bool AllocateTileBuffer(unsigned int Size){ return TileBuffer.Allocate(MainWindow,Size); }
		bool AllocateSpriteBuffer(unsigned int Size){
			if(!SpriteBuffer.Allocate(MainWindow,Size))
				return false;
			SDL_SetColorKey(SpriteBuffer.Surface,SDL_SRCCOLORKEY | SDL_RLEACCEL,
					SDL_MapRGB(SpriteBuffer.Surface->format,0,0xFF,0));
			return true;
		}
		bool LoadTileBuffer(unsigned int DestSlot, SDL_Surface* LoadFrom, Sint16 X, Sint16 Y){ return TileBuffer.Load(DestSlot,LoadFrom,X,Y); }
		bool LoadSpriteBuffer(unsigned int DestSlot, SDL_Surface* LoadFrom, Sint16 X, Sint16 Y){ return SpriteBuffer.Load(DestSlot,LoadFrom,X,Y); }
		bool FlipBuffer();
		bool PrepareNextFrame(const GraphicalData &CurrentState);
		SDL_Surface* GetMainWindow(){ return MainWindow; }
};


#endif // GRAPHICSCORE_H
