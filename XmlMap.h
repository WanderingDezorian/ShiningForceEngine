#ifndef XMLMAP_H
#define XMLMAP_H

#include <SDL/SDL.h>
#include <map>
#include "RapidXml/rapidxml.hpp"
#include "ResourceCore.h"

// Game shall have the following files:
//  Map files - *.tmx, follow "tiled" executable file format
//  Level files - *.xml, indicate a map file, mob text, battle orientation, etc.
//  Game manifest - manifest.xml, lists all levels and entry requirements.

class XmlDoc{
	char* FileBuf; // TODO:  Make static- come up with static deletor
	unsigned int FileBufSize;
protected:
	rapidxml::xml_document<> Doc;
public:
	XmlDoc() : FileBuf(0), FileBufSize(0), Doc() {}
	~XmlDoc(){ if(FileBuf) delete[] FileBuf; }

	void PreAllocateBuffer(unsigned int NewSize);
	bool OpenFile(const char* Filename);
	bool IsOpen()const{ return FileBufSize && (*FileBuf != '\0'); }
	void CloseFile();
	static void ClearAttributeReadErrors();
};

class MapFile : public XmlDoc{
	rapidxml::xml_node<> *Layer;
public:
	MapFile() : Layer(0) {}
	~MapFile(){}
	bool OpenFile(const char* Filename);
	bool SeekToFirstLayer();
	bool NextLayer();
	Point GetLayerSize();
	Point GetBlockerSizeInTiles();
	bool LoadLayerData(unsigned int* Buffer, unsigned int &BufferSize, bool DestructiveLoad = true);
	bool LoadBlockerData(unsigned int* Buffer, unsigned int &BufferSize, bool DestructiveLoad = true); //TODO:  Verify blocker size is less than map max size.
	bool LoadImageData(GraphicsCore &LoadTo, const std::map<unsigned int,unsigned int> &TileAssignments);
};

class LevelFile : public XmlDoc{
	rapidxml::xml_node<> *Layer;
public:
	LevelFile() : Layer(0) {}
	~LevelFile(){}
	bool OpenFile(const char* Filename);
};

class MasterManifest : public XmlDoc{
	rapidxml::xml_node<> *Layer;
public:
	MasterManifest() : Layer(0) {}
	~MasterManifest(){}
	bool OpenFile(const char* Filename);
};

#endif // XMLMAP_H

