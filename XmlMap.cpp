#include "XmlMap.h"
#include "GraphicsCore.h"
#include <errno.h>
#include <fstream>
#include <png.h>

CharBuffer XmlDoc::FileBuf;

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

inline bool ReadNumber(const char* String, int &Value){
	char* EndScan;
	Value = strtol(String, &EndScan, 0);
	return (errno == 0) && (*EndScan == '\0'); // Make sure good scan with no garbage at end
}

inline bool ReadNumber(const char* String, unsigned int &Value){
	char* EndScan;
	Value = strtol(String, &EndScan, 0);
	return (errno == 0) && (*EndScan == '\0'); // Make sure good scan with no garbage at end
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
	FileBuf.Resize(NewSize + 1); // Add extra character for null-termination.
}

bool XmlDoc::PreAllocateBuffer(const char* Filename){
	if(GLOBAL_ZipFile.IsOpen()){
		if(!GLOBAL_ZipFile.ContainsFile(Filename))
			return false;
		PreAllocateBuffer(GLOBAL_ZipFile.Filesize());
		return true;
	}
	std::ifstream fin(Filename);
	if(!fin.is_open())
		return false;
	fin.seekg(0,std::ios_base::end);
	PreAllocateBuffer(fin.tellg());
	fin.close();
	return true;
}


bool XmlDoc::OpenFile(const char* Filename){
	if(FileBuf.Size == 0)
		return false;
	int BytesRead;
	if(GLOBAL_ZipFile.IsOpen()){
		if(!GLOBAL_ZipFile.ContainsFile(Filename))
			return false;
		BytesRead = GLOBAL_ZipFile.UncompressInexact((unsigned char*) FileBuf.Buf,FileBuf.Size);
	}
	else{
		std::ifstream fin(Filename);
		if(!fin.is_open())
			return false;
		fin.read(FileBuf.Buf,FileBuf.Size);
		if(!fin.eof()){ // If we did not reach the end of the file...
			CloseFile();
			return false;
		}
		BytesRead = fin.gcount();
	}
	FileBuf.Buf[BytesRead] = '\0'; //Put in null-terminating zero.
	try{
		Doc.parse<0>(FileBuf.Buf);
	}catch(rapidxml::parse_error &e){
		CloseFile();
		throw e;
		return false;
	}
	return true;
}

void XmlDoc::CloseFile(){
	if(FileBuf.Size != 0)
		FileBuf.Buf[0] = '\0';
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
	if(Layer == 0)
		return false;
	xml_attribute<> *xAttrib = Layer->first_attribute("name");
	if(xAttrib && (strcmp(xAttrib->value(),"Blockers") == 0))
		return NextLayer();
	return true;
}

bool MapFile::NextLayer(){
	if(!Layer)
		return false;
	Layer = Layer->next_sibling("layer");
	if(Layer == 0)
		return false;
	xml_attribute<> *xAttrib = Layer->first_attribute("name");
	if(xAttrib && (strcmp(xAttrib->value(),"Blockers") == 0))
		return NextLayer();
	return true;
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
		const char* ImgSrcName;
		xml_node<> *xFile = xSet->first_node("image");
		if(xFile == 0)
			return false;
		if(!ReadAttribute(xFile,"source",ImgSrcName))
			return false;
		if(strcmp(ImgSrcName,"blockIcons.png") == 0) // This is the blocker source!
			continue;
		if(!(ReadAttribute(xSet,"tilewidth",FirstGid) && FirstGid == GTileSize)) // TODO:  Blockers won't be same width.
			return false;
		if(!(ReadAttribute(xSet,"tileheight",FirstGid) && FirstGid == GTileSize))
			return false;
		if(!ReadAttribute(xSet,"firstgid",FirstGid))
			return false;
		if(!ReadAttribute(xSet,"spacing",Spacing))
			Spacing = 0;

		if(!ReadAttribute(xFile,"width",TileWidth))
			return false;
		TileWidth = (TileWidth + Spacing)/(GTileSize + Spacing);
		if(!ReadAttribute(xFile,"height",NextImageFirstGid)) // Image height in pixels
			return false;
		NextImageFirstGid = (NextImageFirstGid + Spacing)/(GTileSize + Spacing); // Now number of rows of tiles
		NextImageFirstGid = (NextImageFirstGid * TileWidth) + FirstGid; // Now total number of tiles, + FirstGid
		std::map<unsigned int, unsigned int>::const_iterator iAssign = TileAssignments.lower_bound(FirstGid);
		std::map<unsigned int, unsigned int>::const_iterator iEnd = TileAssignments.lower_bound(NextImageFirstGid);
		if(iAssign == iEnd)
			continue;
		SDL_Surface* CurrentTileSet = LoadPng(ImgSrcName);
		if(!CurrentTileSet)
			return false;
		SurfaceGuard TileSetGuard(CurrentTileSet);
		while(iAssign != iEnd){
			unsigned int TileY = (iAssign->first - FirstGid) / TileWidth;
			unsigned int TileX = (iAssign->first - FirstGid) - TileY * TileWidth;
			LoadTo.LoadTileBuffer(iAssign->second,CurrentTileSet,TileX*(GTileSize+Spacing),TileY*(GTileSize+Spacing));
			iAssign++;
		}
	}// End for each tileset
	return true;
}

bool MapFile::LoadBlockerData(unsigned int* Buffer, unsigned int &BufferSize, unsigned int &BlockerGid, bool DestructiveLoad){
	rapidxml::xml_node<> *map = Doc.first_node("map");
	if(map == 0)
		return false;
	rapidxml::xml_node<> *OriginalLayer = Layer;
	bool RetVal = false;
	for(Layer = map->first_node("layer"); Layer != 0; Layer = Layer->next_sibling("layer")){
		xml_attribute<> *xAttrib = Layer->first_attribute("name");
		if(xAttrib && (strcmp(xAttrib->value(),"Blockers") == 0)){
			RetVal = LoadLayerData(Buffer, BufferSize, DestructiveLoad);
			break;
		}
	}
	Layer = OriginalLayer;
	BlockerGid = 0;
	for(xml_node<> *xSet = map->first_node("tileset"); xSet != 0; xSet = xSet->next_sibling("tileset")){
		xml_node<> *xImg = xSet->first_node("image");
		if(xImg == 0)
			continue;
		const char* ImgName;
		if(ReadAttribute(xImg,"source",ImgName) && (strcmp(ImgName,"blockIcons.png") == 0)){
			ReadAttribute(xSet,"firstgid",BlockerGid);
			break;
		}
	}

	return RetVal;
}

Point MapFile::GetBlockerSizeInTiles(){
	rapidxml::xml_node<> *map = Doc.first_node("map");
	if(map == 0)
		return Point(0);
	for(rapidxml::xml_node<> *pLayer = map->first_node("layer"); pLayer != 0; pLayer = pLayer->next_sibling("layer")){
		xml_attribute<> *xAttrib = pLayer->first_attribute("name");
		if(xAttrib && (strcmp(xAttrib->value(),"Blockers") == 0)){
			Point RetVal;
			if(!ReadAttribute(pLayer,"width",RetVal.X))
				return Point(0);
			if(!ReadAttribute(pLayer,"height",RetVal.Y))
				return Point(0);
			return RetVal/2;
		}
	}
	return Point(0);
}

Point MapFile::GetEntryPoint(const char *AnchorName){
	rapidxml::xml_node<> *map = Doc.first_node("map");
	if(map == 0)
		return Point(0);
	rapidxml::xml_node<> *pObj = 0;
	for(rapidxml::xml_node<> *pGroup = map->first_node("objectgroup"); pGroup != 0; pGroup = pGroup->next_sibling("objectgroup")){
		for(pObj = pGroup->first_node("object"); pObj != 0; pObj = pObj->next_sibling("object")){
			const char* Attr;
			if(!ReadAttribute(pObj,"type",Attr))
				continue;
			if(strcmp(Attr,"anchor") == 0){
				if(ReadAttribute(pObj,"name",Attr) && (strcmp(Attr,AnchorName) == 0)) // Anchor names must match exactly
					break;
			}
			else if((strcmp(Attr,"g2anchor") == 0) && ReadAttribute(pObj,"name",Attr)){ // g2anchors should have the tails cut off before comparing
				const char* Split = strchr(Attr,':'); // Check to see if tail is there.  May not be.
				if((Split == 0) && (strcmp(Attr,AnchorName) == 0))
					break;
				if((Split != 0) && (strncmp(Attr,AnchorName,Split-Attr) == 0))
					break;
			}
		}
	}
	if(pObj == 0)
		return Point(0);
	Point RetVal;
	if(!ReadAttribute(pObj,"x",RetVal.X))
		return Point(0);
	if(!ReadAttribute(pObj,"y",RetVal.Y))
		return Point(0);
	RetVal /= PTileSize;
	return RetVal;
}

unsigned int MapFile::GetNumSpecials(){
	rapidxml::xml_node<> *map = Doc.first_node("map");
	if(map == 0)
		return 0;
	unsigned int RetVal = 0;
	for(rapidxml::xml_node<> *pGroup = map->first_node("objectgroup"); pGroup != 0; pGroup = pGroup->next_sibling("objectgroup")){
		for(rapidxml::xml_node<> *pObj = pGroup->first_node("object"); pObj != 0; pObj = pObj->next_sibling("object")){
			const char* Attr;
			if(!ReadAttribute(pObj,"type",Attr))
				continue;
			if((strcmp(Attr,"goto") == 0)
					|| (strcmp(Attr,"gosub") == 0)
					|| (strcmp(Attr,"g2anchor") == 0))
				RetVal++;
		}
	}
	return RetVal;
}

bool MapFile::LoadSpecials(GameData &LoadTo){
	Point BlockerSizeInTiles = GetBlockerSizeInTiles();
	if(BlockerSizeInTiles == 0)
		return false;
	rapidxml::xml_node<> *map = Doc.first_node("map");
	if(map == 0)
		return false;
	LoadTo.SpecialsBufEnd = LoadTo.Specials;
	for(rapidxml::xml_node<> *pGroup = map->first_node("objectgroup"); pGroup != 0; pGroup = pGroup->next_sibling("objectgroup")){
		for(rapidxml::xml_node<> *pObj = pGroup->first_node("object"); pObj != 0; pObj = pObj->next_sibling("object")){
			const char* Attr;
			if(!ReadAttribute(pObj,"type",Attr))
				continue;
			char Type = '\0';
			if(strcmp(Attr,"goto") == 0)
				Type = 'g';
			else if(strcmp(Attr,"g2anchor") == 0)
				Type = 'G';
			else if(strcmp(Attr,"gosub") == 0)
				Type = 's';
			if(Type == '\0')
				continue;
			if(LoadTo.SpecialsBufEnd == LoadTo.Specials + LoadTo.SpecialBufferSize) // No more room
				return false;
			LoadTo.SpecialsBufEnd->Type = Type;
			if(!ReadAttribute(pObj,"x",LoadTo.SpecialsBufEnd->Pos.X))
				return false;
			if(!ReadAttribute(pObj,"y",LoadTo.SpecialsBufEnd->Pos.Y))
				return false;
			if(!ReadAttribute(pObj,"width",LoadTo.SpecialsBufEnd->Range.X))
				return false;
			if(!ReadAttribute(pObj,"height",LoadTo.SpecialsBufEnd->Range.Y))
				return false;
			if(!ReadAttribute(pObj,"name",Attr))
				return false;
			strncpy(LoadTo.SpecialsBufEnd->Data,Attr,32);
			if(LoadTo.SpecialsBufEnd->Data[31] != '\0') // Name too long.
				return false;
			if(Type == 'G'){ // Trim tails on g2anchor
				LoadTo.SpecialsBufEnd->Type = 'g';
				char* Temp = strchr(LoadTo.SpecialsBufEnd->Data,':');
				if(Temp)
					*Temp = '\0';
			}
			LoadTo.SpecialsBufEnd->Pos /= PTileSize;
			LoadTo.SpecialsBufEnd->Range /= PTileSize;
			LoadTo.SpecialsBufEnd->Range.maxEq(1); // Range of zero makes no sense
			if((LoadTo.SpecialsBufEnd->Pos + LoadTo.SpecialsBufEnd->Range).Outside(BlockerSizeInTiles))
				return false;
			// Flag in blockers.
			for(int y = 0; y < LoadTo.SpecialsBufEnd->Range.Y; y++){
				for(int x = 0; x < LoadTo.SpecialsBufEnd->Range.X; x++){
					LoadTo.Blockers[(LoadTo.SpecialsBufEnd->Pos.Y + y) * BlockerSizeInTiles.X
					                + LoadTo.SpecialsBufEnd->Pos.X + x] |= GameData::BLOCKER_SPECIAL;
				}
			}
			LoadTo.SpecialsBufEnd++;
		}
	}
	return true;
}

bool MasterManifest::OpenFile(const char* Filename){
	Levels = 0;
	if(!XmlDoc::OpenFile(Filename))
		return false;
	rapidxml::xml_node<> *game = Doc.first_node("game");
	if(game != 0)
		Levels = game->first_node("levels");
	if(Levels)
		return true;
	XmlDoc::CloseFile();
	return false;
}

bool MasterManifest::ValidateManifest(ZipfileInterface &ToVal){
// 1) Verify all files exist
// 2) Determine maximum file size
// 3) Validate all linkages (todo)
	unsigned int MaxSize = 0;
	for(rapidxml::xml_node<> *iLevel = Levels->first_node(); iLevel != 0; iLevel = iLevel->next_sibling()){
		const char *Name;
		unsigned int Size;
		if(!ReadAttribute(iLevel,"src",Name))
			return false;
		Size = ToVal.Filesize(Name);
		if(Size == 0)
			return false;
		if(Size > MaxSize)
			MaxSize = Size;

		if(!ReadAttribute(iLevel,"map",Name))
			return false;
		Size = ToVal.Filesize(Name);
		if(Size == 0)
			return false;
		if(Size > MaxSize)
			MaxSize = Size;
	}
	XmlDoc::PreAllocateBuffer(MaxSize);
	return true;
}

bool MasterManifest::GetLevelFiles(const char* LevelName, std::string &Src, std::string &Map){
	if(*LevelName == '%'){ // Translate keyword into actual value.
		unsigned int LevelNum;
		if(ReadNumber(LevelName + 1,LevelNum))
			return GetLevelFiles(LevelNum, Src, Map);
		XmlDoc::ClearAttributeReadErrors();
		if(!ReadAttribute(Levels,LevelName+1,LevelName))
			return false;
	}
	for(rapidxml::xml_node<> *iLevel = Levels->first_node(); iLevel != 0; iLevel = iLevel->next_sibling()){
		xml_attribute<> *xAttrib = iLevel->first_attribute("name");
		if(xAttrib && (strcmp(xAttrib->value(),LevelName) == 0)){
			const char *cSrc,*cMap;
			if(!ReadAttribute(iLevel,"src",cSrc))
				return false;
			if(!ReadAttribute(iLevel,"map",cMap))
				return false;
			Src = cSrc;
			Map = cMap;
			return true;
		}
	}
	return false;
}

bool MasterManifest::GetLevelFiles(unsigned int LevelNum, std::string &Src, std::string &Map){
	for(rapidxml::xml_node<> *iLevel = Levels->first_node(); iLevel != 0; iLevel = iLevel->next_sibling()){
		if(LevelNum == 0){
			const char *cSrc,*cMap;
			if(!ReadAttribute(iLevel,"src",cSrc))
				return false;
			if(!ReadAttribute(iLevel,"map",cMap))
				return false;
			Src = cSrc;
			Map = cMap;
			return true;
		}
		LevelNum--;
	}
	return false;
}

unsigned int MasterManifest::GetNumLevels(){
	unsigned int RetVal = 0;
	for(rapidxml::xml_node<> *iLevel = Levels->first_node(); iLevel != 0; iLevel = iLevel->next_sibling())
		RetVal++;
	return RetVal;
}
