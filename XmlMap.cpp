#include "XmlMap.h"
#include <errno.h>
#include <fstream>
#include <png.h>

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

using namespace rapidxml;

void XmlDoc::PreAllocateBuffer(unsigned int NewSize){
	if(NewSize > FileBufSize){
		FileBufSize = 0;
		if(FileBuf)
			delete[] FileBuf;
		FileBuf = new char[NewSize + 1]; // Add extra character for null-termination.
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
	FileBuf[fin.tellg()] = '\0'; //Put in null-terminating zero.
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

void XmlDoc::ClearAttributeReadErrors(){ errno = 0; }

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

bool MapFile::LoadImageData(GraphicsCore &LoadTo, const std::map<unsigned int,unsigned int> &TileAssignments){
	rapidxml::xml_node<> *xMap = Doc.first_node("map");
	if(xMap == 0)
		return false;
	for(xml_node<> *xSet = xMap->first_node("tileset"); xSet != 0; xSet = xSet->next_sibling("tileset")){
		unsigned int FirstGid, NextImageFirstGid, Spacing, TileWidth;
		if(!(ReadAttribute(xSet,"tilewidth",FirstGid) && FirstGid == 24))
			return false;
		if(!(ReadAttribute(xSet,"tileheight",FirstGid) && FirstGid == 24))
			return false;
		if(!ReadAttribute(xSet,"firstgid",FirstGid))
			return false;
		if(!ReadAttribute(xSet,"spacing",Spacing))
			return false;
		xml_node<> *xFile = xSet->first_node("image");
		if(!ReadAttribute(xFile,"width",TileWidth))
			return false;
		TileWidth = (TileWidth + Spacing)/(24 + Spacing);
		if(!ReadAttribute(xFile,"height",NextImageFirstGid)) // Image height in pixels
			return false;
		NextImageFirstGid = (NextImageFirstGid + Spacing)/(24 + Spacing); // Now number of rows of tiles
		NextImageFirstGid = (NextImageFirstGid * TileWidth) + FirstGid; // Now total number of tiles, + FirstGid
		std::map<unsigned int, unsigned int>::const_iterator iAssign = TileAssignments.lower_bound(FirstGid);
		std::map<unsigned int, unsigned int>::const_iterator iEnd = TileAssignments.lower_bound(NextImageFirstGid);
		if(iAssign == iEnd)
			continue;
		xml_attribute<> *xFilename = xFile->first_attribute("source");
		if(!xFilename)
			return false;
		SDL_Surface* CurrentTileSet = LoadPng(xFilename->value());
		if(!CurrentTileSet)
			throw 1;
		SurfaceGuard TileSetGuard(CurrentTileSet);
		while(iAssign != iEnd){
			unsigned int TileY = (iAssign->first - FirstGid) / TileWidth;
			unsigned int TileX = (iAssign->first - FirstGid) - TileY * TileWidth;
			LoadTo.LoadTileBuffer(iAssign->second,CurrentTileSet,TileX*(24+Spacing),TileY*(24+Spacing));
			iAssign++;
		}
	}// End for each tileset
	return true;
}
