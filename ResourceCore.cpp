#include "ResourceCore.h"
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <png.h>
#include <stdlib.h>
#include <errno.h>
#include <vector>
#include <set>
#include <map>
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

inline bool ReadAttribute(rapidxml::xml_node<> *Parent, const char* AttributeName, const char* &Value){
	rapidxml::xml_attribute<> *xAttrib = Parent->first_attribute(AttributeName);
	if(!xAttrib)
		return false;
	Value = xAttrib->value();
	return true;
}

inline bool ReadAttribute(rapidxml::xml_node<> *Parent, const char* AttributeName, unsigned int &Value){
	rapidxml::xml_attribute<> *xAttrib = Parent->first_attribute(AttributeName);
	if(!xAttrib)
		return false;
	char* EndScan;
	Value = strtoul(xAttrib->value(), &EndScan, 0);
	if(errno || (*EndScan != '\0')) // Make sure good scan with no garbage at end
		return false;
	return true;
}

inline bool ReadAttribute(rapidxml::xml_node<> *Parent, const char* AttributeName, int &Value){
	rapidxml::xml_attribute<> *xAttrib = Parent->first_attribute(AttributeName);
	if(!xAttrib)
		return false;
	char* EndScan;
	Value = strtol(xAttrib->value(), &EndScan, 0);
	if(errno || (*EndScan != '\0')) // Make sure good scan with no garbage at end
		return false;
	return true;
}
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

using namespace rapidxml;
using namespace std;

static class b64decoder{
	unsigned char Ring[256];
	void Initialize(){
		for(int i = 0; i < 256; i++)
			Ring[i] = 0xFF;
		for(int i = 'A'; i <= 'Z'; i++)
			Ring[i] = char(i - 'A');
		for(int i = 'a'; i <= 'z'; i++)
			Ring[i] = char(i - 'a' + 26);
		for(int i = '0'; i <= '9'; i++)
			Ring[i] = char(i - '0' + 52);
		Ring['+'] = 62;
		Ring['/'] = 63;
	}
public:
	b64decoder(){ Initialize(); }
	int Decode(unsigned char* ToDecode){ // Will decode in place, returns bits remaining
		unsigned char* iOut = ToDecode;
		unsigned char* iIn = ToDecode;
		while((*iIn == ' ') || (*iIn == '\t') || (*iIn == '\n') || (*iIn == '\r'))
			iIn++;
		while(1){
			// Bits 1-6
			if(Ring[*iIn] == 255)
				break;
			*iOut = Ring[*iIn] << 2;			//xxxxxx00
			iIn++;
			// Bits 7-12
			if(Ring[*iIn] == 255)
				break;
			*iOut |= (Ring[*iIn] & 0x30) >> 4;	//000000xx
			iOut++;
			*iOut = (Ring[*iIn] & 0x0F) << 4;	//xxxx0000
			iIn++;
			// Bits 13-18 (b2:5-12)
			if(Ring[*iIn] == 255)
				break;
			*iOut |= (Ring[*iIn] & 0x3C) >> 2;	//0000xxxx
			iOut++;
			*iOut = (Ring[*iIn] & 0x03) << 6;	//xx000000
			iIn++;
			// Bits 18-24 (b3:3-8)
			if(Ring[*iIn] == 255)
				break;
			*iOut |= Ring[*iIn];
			iOut++;
			iIn++;
		}
		if(*iIn != '=')
			return -1;
		return iOut - ToDecode;
	}
} b64;

int b64decode(unsigned char* ToDecode){
	return b64.Decode(ToDecode);
}

int LoadMap(const char* Filename, GraphicsCore &Core, TileMapping* TileLayers, unsigned int NumLayers){ // Returns number of layers provided
	std::ifstream fin(Filename);
	if(!fin.is_open())
		return -1;
	fin.seekg(0,std::ios_base::end);
	int FileLength = fin.tellg();
	char* FileBuf = new char[FileLength + 1]; // Add extra for '\0' safety cap.
	ArrayGuard<char> Guard_FileBuf(FileBuf);
	fin.seekg(0,std::ios_base::beg);
	fin.read(FileBuf,FileLength);
	FileBuf[FileLength] = '\0';
	int NumLayersLoaded = 0;
	std::map<unsigned int,unsigned int> TileAssignments;
	errno = 0;

	try{
		xml_document<> doc;    // character type defaults to char
		doc.parse<0>(FileBuf);    // 0 means default parse flags
		xml_node<> *xMap = doc.first_node("map");    // 0 means default parse flags
		if(xMap == 0){ throw 1; }
		for(xml_node<> *xLayer = xMap->first_node("layer"); xLayer != 0; xLayer = xLayer->next_sibling("layer"), TileLayers++){
			if(NumLayersLoaded >= NumLayers)
				return NumLayersLoaded;
			NumLayersLoaded++;
			try{
				// Read width
				if(!ReadAttribute(xLayer,"width",TileLayers->SizeX))
					throw 1;
				// Read height
				if(!ReadAttribute(xLayer,"height",TileLayers->SizeY))
					throw 1;
				// Get data
				if(TileLayers->SizeX * TileLayers->SizeY > TileMapping::MaxBufferSize)
					throw 1;
				xml_node<> *xData = xLayer->first_node("data");
				if(!xData){ throw 1; }
				xml_attribute<> *xAttrib = xData->first_attribute("encoding");
				bool Encoded = xData && (strcmp(xAttrib->value(),"base64") == 0);
				xAttrib = xData->first_attribute("compression");
				bool Compressed = xData && (strcmp(xAttrib->value(),"zlib") == 0);
				xml_node<> *xTileBuffer = xData->first_node();
				if(!xTileBuffer->value()){ throw 1; }
				int BufferSize;
				if(Encoded)
					BufferSize = b64.Decode((unsigned char*) xTileBuffer->value());
				else
					BufferSize = strlen(xTileBuffer->value());
				if(Compressed){
					uLongf OutBufferSize = TileLayers->SizeX * TileLayers->SizeY * sizeof(unsigned int);
					const Bytef* InBuf = (const Bytef*) xTileBuffer->value();
					if(uncompress((Bytef*) TileLayers->TileValues, &OutBufferSize, InBuf, BufferSize) != Z_OK){ throw 1; }
				}
				else{
					if(BufferSize > TileMapping::MaxBufferSize)
						BufferSize = TileMapping::MaxBufferSize;
					memcpy(TileLayers->TileValues,xTileBuffer->value(),BufferSize);
				}
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
			}catch(...){
				NumLayersLoaded--;
			}
		} // end for every layer
		// Todo: Zero out remaining layers
		// Load tile images
		// WORKING HERE:  Load in the tiles.
		for(xml_node<> *xSet = xMap->first_node("tileset"); xSet != 0; xSet = xSet->next_sibling("tileset")){
			unsigned int FirstGid, NextImageFirstGid, Spacing, TileWidth;
			if(!(ReadAttribute(xSet,"tilewidth",FirstGid) && FirstGid == 24))
				throw 1;
			if(!(ReadAttribute(xSet,"tileheight",FirstGid) && FirstGid == 24))
				throw 1;
			if(!ReadAttribute(xSet,"firstgid",FirstGid))
				throw 1;
			if(!ReadAttribute(xSet,"spacing",Spacing))
				throw 1;
			xml_node<> *xFile = xSet->first_node("image");
			if(!ReadAttribute(xFile,"width",TileWidth))
				throw 1;
			TileWidth = (TileWidth + Spacing)/(24 + Spacing);
			if(!ReadAttribute(xFile,"height",NextImageFirstGid)) // Image height in pixels
				throw 1;
			NextImageFirstGid = (NextImageFirstGid + Spacing)/(24 + Spacing); // Now number of rows of tiles
			NextImageFirstGid = (NextImageFirstGid * TileWidth) + FirstGid; // Now total number of tiles, + FirstGid
			std::map<unsigned int, unsigned int>::const_iterator iAssign = TileAssignments.lower_bound(FirstGid);
			std::map<unsigned int, unsigned int>::const_iterator iEnd = TileAssignments.lower_bound(NextImageFirstGid);
			if(iAssign == iEnd)
				continue;
			xml_attribute<> *xFilename = xFile->first_attribute("source");
			if(!xFilename)
				throw 1;
			SDL_Surface* CurrentTileSet = LoadPng(xFilename->value());
			if(!CurrentTileSet)
				throw 1;
			SurfaceGuard TileSetGuard(CurrentTileSet);
			while(iAssign != iEnd){
				unsigned int TileY = (iAssign->first - FirstGid) / TileWidth;
				unsigned int TileX = (iAssign->first - FirstGid) - TileY * TileWidth;
				Core.LoadTileBuffer(iAssign->second,CurrentTileSet,TileX*(24+Spacing),TileY*(24+Spacing));
				iAssign++;
			}
			for(xml_node<> *xLayer = xMap->first_node("objectgroup"); xLayer != 0; xLayer = xLayer->next_sibling("objectgroup")){
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
			}
		}// End for each tileset
	}catch(...){
		return 0;
	}
	return NumLayersLoaded;
}

bool GetMapInfo(const char* Filename, unsigned int &NumLayers, unsigned int &MaxSizeXinTiles, unsigned int &MaxSizeYinTiles,unsigned int &NumUniqueTiles){
	std::ifstream fin(Filename);
	if(!fin.is_open())
		return false;
	fin.seekg(0,std::ios_base::end);
	int FileLength = fin.tellg();
	char* FileBuf = new char[FileLength + 1]; // Add extra for '\0' safety cap.
	ArrayGuard<char> Guard_FileBuf(FileBuf);
	fin.seekg(0,std::ios_base::beg);
	fin.read(FileBuf,FileLength);
	FileBuf[FileLength] = '\0';
	std::set<unsigned int> UniqueTiles;
	unsigned int LocalNumLayers = 0;
	MaxSizeXinTiles = 0;
	MaxSizeYinTiles = 0;
	errno = 0;
	try{
		xml_document<> doc;    // character type defaults to char
		doc.parse<0>(FileBuf);    // 0 means default parse flags
		xml_node<> *xMap = doc.first_node("map");    // 0 means default parse flags
		if(xMap == 0){ return false; }
		for(xml_node<> *xLayer = xMap->first_node("layer"); xLayer != 0; xLayer = xLayer->next_sibling("layer")){
			LocalNumLayers += 1;
			xml_attribute<> *xAttrib = xLayer->first_attribute("width");
			if(!xAttrib)
				return false;
			char* EndScan;
			unsigned int SizeX = strtoul(xAttrib->value(), &EndScan, 0);
			if(errno || (*EndScan != '\0')) // Make sure good scan with no garbage at end
				return false;
			StoreMax(MaxSizeXinTiles, SizeX);

			// Read height
			xAttrib = xLayer->first_attribute("height");
			if(!xAttrib)
				return false;
			unsigned int SizeY = strtoul(xAttrib->value(), &EndScan, 0);
			if(errno || (*EndScan != '\0')) // Make sure good scan with no garbage at end
				return false;
			StoreMax(MaxSizeYinTiles, SizeY);

			// Get data
			xml_node<> *xData = xLayer->first_node("data");
			if(!xData)
				return false;
			xAttrib = xData->first_attribute("encoding");
			bool Encoded = xData && (strcmp(xAttrib->value(),"base64") == 0);
			xAttrib = xData->first_attribute("compression");
			bool Compressed = xData && (strcmp(xAttrib->value(),"zlib") == 0);
			xml_node<> *xTileBuffer = xData->first_node();
			if(!xTileBuffer->value()){ throw 1; }
			int BufferSize;
			if(Encoded)
				BufferSize = b64.Decode((unsigned char*) xTileBuffer->value());
			else
				BufferSize = strlen(xTileBuffer->value());
			if(Compressed){
				std::vector<unsigned int> Buffer(SizeX*SizeY);
				const Bytef* InBuf = (const Bytef*) xTileBuffer->value();
				uLong OutBufSize = SizeX*SizeY*sizeof(unsigned int);
				if(Z_OK != uncompress((Bytef*) (&Buffer[0]), &OutBufSize, InBuf, BufferSize))
					return false;
				UniqueTiles.insert(Buffer.begin(),Buffer.end());
			}
			else{
				UniqueTiles.insert((unsigned int*)xTileBuffer->value(),(unsigned int*) (xTileBuffer->value() + BufferSize));
			}
		}
	}catch(...){
		return false;
	}
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

void XmlDoc::PreAllocateBuffer(unsigned int NewSize){
	if(NewSize > FileBufSize){
		FileBufSize = 0;
		if(FileBuf)
			delete[] FileBuf;
		FileBuf = new char[NewSize];
		FileBufSize = NewSize;
	}
}

bool XmlDoc::OpenFile(const char* Filename){
	if(FileBufSize == 0)
		return false;
	std::ifstream fin(Filename);
	if(!fin.is_open())
		return false;
	Doc.clear();
	fin.read(FileBuf,FileBufSize);
	if(fin.fail()){
		CloseFile();
		return false;
	}
	try{
		Doc.parse<0>(FileBuf);
	}catch(...){
		CloseFile();
		return false;
	}
	return true;
}

void XmlDoc::CloseFile(){
	if(FileBufSize != 0)
		FileBuf[0] = '\0';
	Doc.clear();
}

bool MapFile::OpenFile(const char* Filename){
	Layer = 0;
	if(!XmlDoc::OpenFile(Filename))
		return false;
	return SeekToFirstLayer();
}

bool MapFile::SeekToFirstLayer(){
	rapidxml::xml_node<> *map = Doc.first_node("map");
	if(map == 0)
		return false;
	Layer = map->first_node("layer");
	return Layer != 0;
}

bool MapFile::NextLayer(){
	if(!Layer)
		return false;
	Layer = Layer->next_sibling("layer");
	return Layer != 0;
}
Point MapFile::GetLayerSize(){
	if(!Layer)
		return Point(0);
	Point RetVal;
	if(!ReadAttribute(Layer,"width",RetVal.X))
		return Point(0);
	if(!ReadAttribute(Layer,"height",RetVal.Y))
		return Point(0);
	return RetVal;
}

bool MapFile::LoadLayerData(unsigned int* Buffer, unsigned int &BufferSize, bool DestructiveLoad){
	if(!Layer)
		return false;
	uLongf BufferSizeInBytes = BufferSize * sizeof(unsigned int);
	rapidxml::xml_node<> *xData = Layer->first_node("data");
	if(!xData)
		return false;
	xml_attribute<> *xAttrib = xData->first_attribute("encoding");
	bool Encoded = xAttrib && (strcmp(xAttrib->value(),"base64") == 0);
	xAttrib = xData->first_attribute("compression");
	bool Compressed = xAttrib && (strcmp(xAttrib->value(),"zlib") == 0);

	xml_node<> *xTileBuffer = xData->first_node();
	if(!xTileBuffer->value())
		return false;
	unsigned int EncodedBufferSize;
	if(Encoded){
		if(DestructiveLoad)
			EncodedBufferSize = b64.Decode((unsigned char*) xTileBuffer->value());
		else
			return false; // TODO:  Implement non-destructive loads!
	}
	else
		EncodedBufferSize = strlen(xTileBuffer->value());
	if(Compressed){
		const Bytef* InBuf = (const Bytef*) xTileBuffer->value();
		if(uncompress((Bytef*) Buffer, &BufferSizeInBytes, InBuf, EncodedBufferSize) != Z_OK){ throw 1; }
	}
	else{
		if(BufferSizeInBytes > EncodedBufferSize)
			BufferSizeInBytes = EncodedBufferSize;
		memcpy(Buffer,xTileBuffer->value(),BufferSizeInBytes);
	}
	// BufferSize = ceil( BufferSizeInBytes / 2 )
	BufferSize = BufferSizeInBytes >> 2;
	if(BufferSizeInBytes & 0x03)
		BufferSize++;
	// Todo:  Endian swap if necessary
	return true;
}
