import base64, zlib

def Update(Data, FirstGid = 1, TileMapWidth = 8, MapWidth = 100, NewGid = 0):
 OutData = [0] * (len(Data) * 4);
 if(NewGid == 0):
  NewGid = FirstGid;
 for [i,val] in enumerate(Data):
  Byte = i%4;
  if((Byte % 4) != 0):
   continue;
  i = i // 4;
  val -= FirstGid;
  OutTile = ((val % TileMapWidth)*2) + ((val // TileMapWidth)*TileMapWidth*4) + NewGid;
  OutPos = ((i % MapWidth)*2) + ((i // MapWidth)*MapWidth*4);
  OutData[(OutPos)*4] = OutTile;
  OutData[(OutPos + 1)*4] = OutTile + 1;
  OutData[(OutPos + 2*MapWidth)*4] = OutTile + 2*TileMapWidth;
  OutData[(OutPos + 2*MapWidth + 1)*4] = OutTile + 2*TileMapWidth + 1;
 for [i,val] in enumerate(OutData):
  if(val >= 256):
   OutData[i-1] = val // 256;
   OutData[i] = val % 256;

def ExpandMap(RawIn, FirstGid = 1, TileMapWidth = 8, MapWidth = 100, NewGid = 0):
 Data = zlib.decompress(base64.decodestring(RawIn));
 OutData = Update(Data,FirstGid,TileMapWidth,MapWidth,NewGid);
 return base64.encodestring(zlib.compress(bytes(OutData)))

def ExpandBlockers(RawIn, FirstGid = 1, TileMapWidth = 8, MapWidth = 100, NewGid = 0):
 Data = zlib.decompress(base64.decodestring(RawIn));
 
 OutData = Update(Data,FirstGid,TileMapWidth,MapWidth,NewGid);
 return base64.encodestring(zlib.compress(bytes(OutData)))
