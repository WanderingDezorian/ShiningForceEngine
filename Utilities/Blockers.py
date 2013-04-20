import base64, zlib
import xml.dom.minidom as xml

BLOCK_UP = 0x01;
BLOCK_DOWN = 0x02;
BLOCK_LEFT = 0x04;
BLOCK_RIGHT = 0x08;
BLOCK_ALL = 0x0F;

BLOCK_TMX2FLAG = [BLOCK_UP|BLOCK_LEFT,BLOCK_UP,BLOCK_UP|BLOCK_RIGHT,BLOCK_UP|BLOCK_DOWN,
  BLOCK_LEFT,BLOCK_ALL,BLOCK_RIGHT,BLOCK_LEFT|BLOCK_RIGHT,
  BLOCK_DOWN|BLOCK_LEFT,BLOCK_DOWN,BLOCK_DOWN|BLOCK_RIGHT,0,
  BLOCK_ALL&(~BLOCK_LEFT),BLOCK_ALL&(~BLOCK_UP),BLOCK_ALL&(~BLOCK_DOWN),BLOCK_ALL&(~BLOCK_RIGHT)];

BLOCK_FLAG2TMX = [0]*16;
for i in range(16):
 BLOCK_FLAG2TMX[i] = BLOCK_TMX2FLAG.index(i);

def CompressData(Ints):
 b = b'';
 for i in Ints:
  b += bytes([(i)&0xFF,(i>>8)&0xFF,(i>>16)&0xFF,(i>>24)&0xFF])
 return base64.encodestring(zlib.compress(b)).decode('utf-8');

def UncompressData(Bytes):
 b = zlib.decompress(base64.decodestring(Bytes));
 Ints = [0]*(len(b)//4);
 for i in range(len(Ints)):
  Ints[i] = (b[i*4]) + (b[i*4+1]<<8) + (b[i*4+2]<<16) + (b[i*4+3]<<24);
 return Ints;

def ParseRules(RulesFile):
 doc = xml.parse(RulesFile);
 Sources = doc.getElementsByTagName('tileset');
 if(len(Sources) != 2): 0/0;
 if(Sources[0].getAttribute('name') == 'SingleBlocker'):
  Sources.reverse();
 SourceImage = Sources[0].getElementsByTagName('image');
 FirstGid = int(Sources[0].getAttribute('firstgid'));
 if(len(SourceImage) != 1): 0/0;
 SourceImage = SourceImage[0];
 Source = (SourceImage.getAttribute('source'),SourceImage.getAttribute('width'),SourceImage.getAttribute('height'))
 Layers = doc.getElementsByTagName('data')
 if(len(Layers) != 2): 0/0;
 if(Layers[0].parentNode.getAttribute('name') == 'Blockers'):
  Layers.reverse(); #Layer zero is now sources, Layer one is now data.
 Tiles = UncompressData(Layers[0].childNodes[0].data.strip().encode());
 Blockers = UncompressData(Layers[1].childNodes[0].data.strip().encode());
 WidthInTiles = int(Layers[0].parentNode.getAttribute('width'));
 m = dict();
 for (i,x) in enumerate(Tiles):
  if(x == 0):
   continue;
  val = m.get(x-FirstGid,0);
  if(Blockers[i]): val |= BLOCK_ALL;
  if(Blockers[i-1]): val |= BLOCK_LEFT;
  if(Blockers[i+1]): val |= BLOCK_RIGHT;
  if(Blockers[i-WidthInTiles]): val |= BLOCK_UP;
  if(Blockers[i+WidthInTiles]): val |= BLOCK_DOWN;
  m[x-FirstGid] = val;
 return (m,Source)

def ExplodedMapData(Width,Height,FirstGid = 2):
 Ints = [0] * (Width * Height * 3 * 3);
 for x in range(Width):
  for y in range(Height):
   Ints[x*3+1 + (y*3+1)*Width*3] = x + y*Width + FirstGid;
 return CompressData(Ints);

def GenerateBlockerBase(ImageName, ImageSizeX, ImageSizeY, TileSizeX = 12, TileSizeY = 12, Spacing=0):
 OutName = ImageName[:ImageName.rfind('.')] + '.Blockers.tmx';
 Width = (ImageSizeX // TileSizeX); 
 Height = (ImageSizeY // TileSizeY);
 fout = open(OutName,'w');
 fout.write(r'<?xml version="1.0" encoding="UTF-8"?>'+'\n');
 fout.write(r'<map version="1.0" orientation="orthogonal" width="{0}" height="{1}" tilewidth="{2}" tileheight="{3}">'.format(Width*3,Height*3,TileSizeX, TileSizeY)+'\n');
 fout.write(r' <tileset firstgid="1" name="SingleBlocker" tilewidth="12" tileheight="12" spacing="0">'+'\n');
 fout.write(r'  <image source="SingleBlocker.png" trans="00ff00" width="12" height="12"/>'+'\n');
 fout.write(r' </tileset>'+'\n');
 fout.write(r' <tileset firstgid="2" name="{0}" tilewidth="{1}" tileheight="{2}" spacing="{3}">'.format(ImageName,TileSizeX,TileSizeY,Spacing)+'\n');
 fout.write(r'  <image source="{0}" width="{1}" height="{2}"/>'.format(ImageName,ImageSizeX,ImageSizeY)+'\n');
 fout.write(r' </tileset>'+'\n');
 fout.write(r' <layer name="Tile Layer 1" width="{0}" height="{1}">'.format(Width*3,Height*3)+'\n');
 fout.write(r'  <data encoding="base64" compression="zlib">'+'\n');
 fout.write(r'   ' + ExplodedMapData(Width,Height));
 fout.write(r'  </data>'+'\n');
 fout.write(r' </layer>'+'\n');
 fout.write(r' <layer name="Blockers" width="{0}" height="{1}">'.format(Width*3,Height*3)+'\n');
 fout.write(r'  <data encoding="base64" compression="zlib">'+'\n');
 fout.write(r'   ' + CompressData([0]*(Width*Height*3*3)));
 fout.write(r'  </data>'+'\n');
 fout.write(r' </layer>'+'\n');
 fout.write(r'</map>'+'\n');

def AutoMap(Map,Rules):
 (m,Source) = ParseRules(Rules);
 doc = xml.parse(Map);
 # --- Determine which GIDs to update.
 Gid = None;
 BlockerGid = None;
 for x in doc.getElementsByTagName('tileset'):
  img = x.getElementsByTagName('image')[0]; 
  if(img.getAttribute('source') == Source[0]):
   Gid = int(x.getAttribute('firstgid'));
  if(img.getAttribute('source') == 'blockIcons.png'):
   BlockerGid = int(x.getAttribute('firstgid'));
 if(Gid == None): 0/0;
 if(BlockerGid == None): 0/0;
 # --- Identify blocker layer
 Layers = doc.getElementsByTagName('layer')
 BLayer = None;
 for (i,x) in enumerate(Layers):
  if(x.getAttribute('name') == 'Blockers'):
   BLayer = Layers.pop(i);
   break;
 if(BLayer == None): 0/0;
 BWidth = int(BLayer.getAttribute('width'));
 BHeight = int(BLayer.getAttribute('height'));
 BlockerLayer = UncompressData(BLayer.getElementsByTagName('data')[0].childNodes[0].data.strip().encode());
 for (i,x) in enumerate(BlockerLayer):
  if((x >= BlockerGid) and (x < BlockerGid + 16)):
   BlockerLayer[i] = BLOCK_TMX2FLAG[x-BlockerGid]; #Translate from TMX to flags
  else:
   BlockerLayer[i] = 0;
 # --- Scan all other layers one at a time for target tiles
 for L in Layers:
  LWidth = int(L.getAttribute('width'));
  Data = UncompressData(L.getElementsByTagName('data')[0].childNodes[0].data.strip().encode());
  for (TileIdx,Tile) in enumerate(Data):
   if((Tile - Gid) in m):
    val = m[Tile - Gid];
    Bx = (TileIdx % LWidth) & (~1);
    By = ((TileIdx // LWidth) | 1);
    BIdx = Bx + By * BWidth;
    BlockerLayer[BIdx] |= val;
    if((val != 0) and (val != BLOCK_ALL)): #Mirror as appropriate
     if((val & BLOCK_UP) and (By != 0)): BlockerLayer[BIdx - 2*BWidth] |= BLOCK_DOWN;
     if((val & BLOCK_DOWN) and (By < BHeight-2)): BlockerLayer[BIdx + 2*BWidth] |= BLOCK_UP;
     if((val & BLOCK_LEFT) and (Bx != 0)): BlockerLayer[BIdx - 2] |= BLOCK_RIGHT;
     if((val & BLOCK_RIGHT) and (Bx < BWidth-2)): BlockerLayer[BIdx + 2] |= BLOCK_LEFT;
 # --- Update data and write it out.
 for (i,x) in enumerate(BlockerLayer):
  if((x >= 0) and (x < 16)):
   BlockerLayer[i] = BLOCK_FLAG2TMX[x] + BlockerGid; #Translate back to TMX
  else:
   BlockerLayer[i] = BLOCK_FLAG2TMX[0] + BlockerGid;
 BLayer.getElementsByTagName('data')[0].childNodes[0].replaceWholeText('\n' + CompressData(BlockerLayer) + '\n');
 fout = open(Map+'.autoBlock.tmx','w');
 doc.writexml(fout);

def ExpandMap(RawIn, FirstGid = 1, TileMapWidth = 8, MapWidth = 100, NewGid = 0):
 DataIn = UncompressData(RawIn);
 DataOut = [0]*(len(DataIn)*4);
 for (i,val) in enumerate(DataIn):
  val -= FirstGid;
  OutVal = ((val % TileMapWidth)*2) + ((val // TileMapWidth)*TileMapWidth*4) + FirstGid;
  x = i%MapWidth;
  y = i//MapWidth;
  ni = 2*x + (2*y)*(MapWidth*2);
  DataOut[ni] = OutVal;
  DataOut[ni+1] = OutVal + 1;
  DataOut[ni+2*MapWidth] = OutVal + 2*TileMapWidth;
  DataOut[ni+2*MapWidth+1] = OutVal + 2*TileMapWidth + 1;
 return CompressData(DataOut);