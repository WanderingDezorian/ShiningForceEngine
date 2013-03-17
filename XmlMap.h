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

/*
 * <game Name= Author=
 *  <intro Format=
 *  <title Image = Flicker=
 *  <levels>
 *  	<town src= map=>
 *  	<battle src= map=>
 */

struct CharBuffer{
	char* Buf; // TODO:  Make static- come up with static deletor
	unsigned int Size;
public:
	CharBuffer() : Buf(0), Size(0) {}
	~CharBuffer(){ if(Buf) delete[] Buf; }
	void Resize(unsigned int NewSize){
		if(Size < NewSize){
			if(Buf)
				delete[] Buf;
			Buf = 0;
			Buf = new char[NewSize];
			Size = NewSize;
		}
	}
};

class XmlDoc{
	static CharBuffer FileBuf;
protected:
	rapidxml::xml_document<> Doc;
public:
	XmlDoc() : Doc() {}
	~XmlDoc(){}

	static void PreAllocateBuffer(unsigned int NewSize);
	static bool PreAllocateBuffer(const char* Filename);
	bool OpenFile(const char* Filename);
	bool IsOpen()const{ return (FileBuf.Size && (FileBuf.Buf[0] != '\0')); }
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
	rapidxml::xml_node<> *Levels;
/*	struct MapNotes{
		bool Searched;
		std::set<std::string> Warps;
	};
	std::map<std::string,std::string> ExistingTags;
	std::map<std::string,std::string> NeededTags;*/
public:
	MasterManifest() : Levels(0) {}
	~MasterManifest(){}
	bool OpenFile(const char* Filename);

	bool ValidateManifest(ZipfileInterface &ToVal);
	// 1) Verify all files exist
	// 2) Determine maximum file size
	// 3) Validate all linkages (todo)

	bool GetLevelFiles(const char* LevelName, std::string &Src, std::string &Map);
	bool GetLevelFiles(unsigned int LevelNum, std::string &Src, std::string &Map);
	unsigned int GetNumLevels();
};

#endif // XMLMAP_H

