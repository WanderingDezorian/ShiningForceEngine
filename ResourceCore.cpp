#include "ResourceCore.h"
#include "XmlMap.h"
#include <fstream>
#include <png.h>
#include <set>
///////////////////////////// Guard classes.  Makes loading structures exception safe.

template<class T> inline T MAX(const T &A, const T &B){ return (A > B) ? A : B; }
template<class T> inline void StoreMax(T &A, const T &B){ if(B > A) A = B; }

class PngGuard{
	png_struct *PngStruct;
	png_info *StartInfo;
	png_info *EndInfo;
public:
	PngGuard(png_struct *ToGuard = 0) : PngStruct(ToGuard), StartInfo(0), EndInfo(0){}
	~PngGuard(){ if(PngStruct || StartInfo || EndInfo) png_destroy_read_struct(&PngStruct,&StartInfo,&EndInfo); }
	void GuardPngStruct(png_struct* ToGuard){ PngStruct = ToGuard; }
	void GuardStartInfo(png_info* ToGuard){ StartInfo = ToGuard; }
	void GuardEndInfo(png_info* ToGuard){ EndInfo = ToGuard; }
	void StopGuarding(){ PngStruct = 0; StartInfo = 0; EndInfo = 0; }
};

template<class T> class ArrayGuard{
	T* myPointer;
public:
	ArrayGuard(T *ToGuard = 0) : myPointer(ToGuard){}
	~ArrayGuard(){ if(myPointer) delete[](myPointer); }
	void StartGuard(T* ToGuard){ myPointer = ToGuard; }
	void StopGuarding(){ myPointer = 0; }
};


///////////////////////////// Top level functions

SDL_Surface* LoadPng(const char* Filename){ // Loads to a software image
	//Prepare the guards - prepare here so all are called by lngjmp
	FileGuard guardfin;
	PngGuard guardPng;
	SurfaceGuard guardRetVal;

	//Open the file
	FILE *fin = fopen(Filename, "rb");
	guardfin.GuardFile(fin);
	if (!fin)
		return 0;

	//Verify the header
	{
		png_byte Header[8];
		fread(Header,1,8,fin);
		if(png_sig_cmp(Header,0,8) != 0)
			return 0; // File closing handled by guard.
	}
	//Initialize structures
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	guardPng.GuardPngStruct(png_ptr);
	if (!png_ptr)
		return 0; // File closing handled by guard
	png_infop info_ptr = png_create_info_struct(png_ptr);
	guardPng.GuardStartInfo(info_ptr);
	if (!info_ptr)
		return 0; // File and png closing handled by guards
	png_infop end_info = png_create_info_struct(png_ptr);
	guardPng.GuardEndInfo(end_info);
	if (!end_info)
		return 0; // File and png closing handled by guards
	if (setjmp(png_jmpbuf(png_ptr)))
		return 0; // If there's an error, jump here.  File and png closing still handled by guards

	// Read the png header
	png_init_io(png_ptr,fin);
	png_set_sig_bytes(png_ptr,8);
	png_read_info(png_ptr,info_ptr);
	unsigned int width = png_get_image_width(png_ptr, info_ptr);
	unsigned int height = png_get_image_height(png_ptr, info_ptr);
	int bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	int color_type = png_get_color_type(png_ptr, info_ptr);

	// Modify so pixel data is RGBA
	if (color_type == PNG_COLOR_TYPE_PALETTE){ // palette -> rgb
		png_set_palette_to_rgb(png_ptr);
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
		png_set_add_alpha(png_ptr, 255, PNG_FILLER_BEFORE);
#else
		png_set_add_alpha(png_ptr, 255, PNG_FILLER_AFTER);
#endif
	}
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) // grayscale 2,4 -> rgba
		png_set_tRNS_to_alpha(png_ptr);
	else if (color_type == PNG_COLOR_TYPE_RGB ||	color_type == PNG_COLOR_TYPE_GRAY)
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
		png_set_add_alpha(png_ptr, 255, PNG_FILLER_BEFORE);
#else
		png_set_add_alpha(png_ptr, 255, PNG_FILLER_AFTER);
#endif
	else
		png_set_swap_alpha(png_ptr);
	if (bit_depth == 16)
		png_set_strip_16(png_ptr); // Bitdepth 16 -> Bitdepth 8
	if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png_ptr);
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	png_set_bgr(png_ptr); // I think this is a low-endian/high-endian swap issue.  Images need to be ABGR.
#endif

	//Prepare the return value
	SDL_Surface* RetVal = SDL_CreateRGBSurface(0,width,height,32,0xFF000000,0x00FF0000,0x0000FF00,0x000000FF);
	guardRetVal.GuardSurface(RetVal);
	if(SDL_MUSTLOCK(RetVal))
		SDL_LockSurface(RetVal);
	std::vector<png_byte*> RowPointers(height);
	for(unsigned int i = 0; i < height; i++)
		RowPointers[i] = (png_byte*) RetVal->pixels + RetVal->pitch * i;

	//Load the data

	png_read_image(png_ptr, &RowPointers.at(0));
	if(SDL_MUSTLOCK(RetVal))
		SDL_UnlockSurface(RetVal);
	png_read_end(png_ptr,end_info);
	SDL_SetAlpha(RetVal, 0, SDL_ALPHA_OPAQUE);
	//Return
	guardRetVal.StopGuarding();
	return RetVal;
}

using namespace std;

int LoadMap(const char* Filename, GraphicsCore &Core, TileMapping* TileLayers, unsigned int NumLayers){ // Returns number of layers provided
	std::ifstream fin(Filename);
	if(!fin.is_open())
		return -1;
	fin.seekg(0,std::ios_base::end);
	MapFile MyMap;
	MyMap.PreAllocateBuffer(fin.tellg());
	fin.close();

	if(!MyMap.OpenFile(Filename))
		return false;
	int NumLayersLoaded = 0;
	std::map<unsigned int,unsigned int> TileAssignments;
	MyMap.ClearAttributeReadErrors();
	Point LayerSize;
	do{
		if(NumLayersLoaded >= NumLayers)
			return NumLayersLoaded;
		NumLayersLoaded++;

		// Read dimensions
		LayerSize = MyMap.GetLayerSize();
		TileLayers->SizeX = LayerSize.X;
		TileLayers->SizeY = LayerSize.Y;
		unsigned int LayerNumEl = LayerSize.X * LayerSize.Y;
		if(LayerNumEl == 0)
			return false;

		// Get data
		if(LayerNumEl > TileMapping::MaxBufferSize)
			return false;
		if(!MyMap.LoadLayerData(TileLayers->TileValues,LayerNumEl))
			return false;

		// Todo:  Endian swap if necessary
		// TODO:  Remove dynamically allocated map
		unsigned int *EndVals = TileLayers->TileValues + TileLayers->SizeX * TileLayers->SizeY;
		for(unsigned int *TileVal = TileLayers->TileValues; TileVal < EndVals; TileVal++){
			if(TileAssignments.count(*TileVal))
				*TileVal = TileAssignments[*TileVal];
			else{
				unsigned int NewVal = TileAssignments.size();
				TileAssignments[*TileVal] = NewVal;
				*TileVal = NewVal;
			}
		}
		TileLayers++;
	}while(MyMap.NextLayer());
		// Todo: Zero out remaining layers
		// Load tile images
		// WORKING HERE:  Load in the tiles.
	if(!MyMap.LoadImageData(Core,TileAssignments))
		return -1;

/*		for(xml_node<> *xLayer = xMap->first_node("objectgroup"); xLayer != 0; xLayer = xLayer->next_sibling("objectgroup")){
			for(xml_node<> *xObj = xLayer->first_node("object"); xObj != 0; xObj = xObj->next_sibling("object")){
				const char* Type = 0;
				unsigned int x = 0, y = 0, w = 0, h = 0;
				ReadAttribute(xObj,"type",Type);
				ReadAttribute(xObj,"x",x);
				ReadAttribute(xObj,"y",y);
				ReadAttribute(xObj,"width",w);
				ReadAttribute(xObj,"height",h);
//					switch(Type){ // TODO:  Use lowercase comparisons
//					case "blocker":
//
//
//					}
			}
		}*/
	return NumLayersLoaded;
}

bool GetMapInfo(const char* Filename, unsigned int &NumLayers, unsigned int &MaxSizeXinTiles, unsigned int &MaxSizeYinTiles,unsigned int &NumUniqueTiles){
	std::ifstream fin(Filename);
	if(!fin.is_open())
		return false;
	fin.seekg(0,std::ios_base::end);
	MapFile MyMap;
	MyMap.PreAllocateBuffer(fin.tellg());
	fin.close();

	if(!MyMap.OpenFile(Filename))
		return false;
	MyMap.ClearAttributeReadErrors();

	std::set<unsigned int> UniqueTiles;
	unsigned int LocalNumLayers = 0;
	MaxSizeXinTiles = 0;
	MaxSizeYinTiles = 0;
	Point LayerSize;
	unsigned int LayerNumEl;
	do{
		LocalNumLayers += 1;
		LayerSize = MyMap.GetLayerSize();
		LayerNumEl = LayerSize.X * LayerSize.Y;
		if(LayerNumEl == 0)
			return false;
		StoreMax(MaxSizeXinTiles, LayerSize.X);
		StoreMax(MaxSizeYinTiles, LayerSize.Y);
		std::vector<unsigned int> Buffer(LayerNumEl);
		if(!MyMap.LoadLayerData(&Buffer[0],LayerNumEl))
			return false;
		UniqueTiles.insert(Buffer.begin(),Buffer.begin() + LayerNumEl);
	}while(MyMap.NextLayer());
	if(LocalNumLayers > NumLayers)
		NumLayers = LocalNumLayers;
	if(NumUniqueTiles < UniqueTiles.size())
		NumUniqueTiles = UniqueTiles.size();
	return true;
}

bool InitializeResources(const char* MapFilename, GraphicsCore& Core, GameState &Data){
	bool AllResourcesLoadedCorrectly = true;
	unsigned int NumLayers = 0, MaxSizeX = 0, MaxSizeY = 0, NumUniqueTiles = 0;
	AllResourcesLoadedCorrectly &= GetMapInfo("TestMap1.tmx", NumLayers, MaxSizeX, MaxSizeY, NumUniqueTiles);

	if(!AllResourcesLoadedCorrectly)
		return false;
	try{ // Catch allocation errors
		if(!Core.AllocateTileBuffer(NumUniqueTiles))
			return false;
		if(!Data.Data.Initialize(MaxSizeX,MaxSizeY,1)) // TODO:  Don't hardcode max num mobs
			return false;
		Data.Graphics.AllocateTileLayers(NumLayers);
		TileMapping::MaxBufferSize = MaxSizeX * MaxSizeY;
		for(unsigned int i = 0; i < NumLayers; i++){
			if(!Data.Graphics.TileLayers[i].Allocate(TileMapping::MaxBufferSize))
				return false;
		}
	}catch(...){
		return false;
	}

	// TODO:  Determine maximum number of sprites
	if(!Core.AllocateSpriteBuffer(8))
		return false;
	return true;
}
