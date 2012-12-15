#ifndef XMLMAP_H
#define XMLMAP_H

#include <SDL/SDL.h>
#include <map>
#include "RapidXml/rapidxml.hpp"
#include "ResourceCore.h"

class XmlDoc{
	char* FileBuf;
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
	bool LoadLayerData(unsigned int* Buffer, unsigned int &BufferSize, bool DestructiveLoad = true);
	bool LoadImageData(GraphicsCore &LoadTo, const std::map<unsigned int,unsigned int> &TileAssignments);
};

#endif // XMLMAP_H

