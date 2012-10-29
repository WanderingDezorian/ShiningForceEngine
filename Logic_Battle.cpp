#include "main.h"
#include "ResourceCore.h"

bool Initialize_Battle(GraphicsCore &GCore, GameState &MainGameState, std::vector<std::string> &TileFilenames){
//	MainGameState.Graphics.TileLayers.resize(1);
//	TileMapping *BaseMapping = &(MainGameState.Graphics.TileLayers[0]);
//	TileFilenames.push_back("WhiteTile.png");
//	TileFilenames.push_back("GrayTile.png");
/*	BaseMapping->SizeX = 15;
	BaseMapping->SizeY = 10;
	BaseMapping->Camera.TileX = 0;
	BaseMapping->Camera.TileY = 0;
	BaseMapping->Camera.SubX = 20;
	BaseMapping->Camera.SubY = 0;
	BaseMapping->MainBufferOffset = 0;
	BaseMapping->TileValues.resize(10*15);
	for(int x = 0; x < 15; x++)
		for(int y = 0; y < 10; y++)
			BaseMapping->TileValues[x + BaseMapping->SizeX * y] = (x & 0x1) ^ (y & 0x1);
*/
	LoadMap("TestMap1.tmx", GCore, MainGameState.Graphics.TileLayers, MainGameState.Graphics.NumTileLayers); // Returns number of layers provided
	MainGameState.Graphics.SpriteLayerDepth = 1;
/*	Sprite mySprite;
	mySprite.RootBufferOffset = 2;
	mySprite.OrientationBufferOffset = 0;
	mySprite.OrientationBufferSize = 1;
	mySprite.CurrentOffset = 0;
	mySprite.UpdatePattern = Sprite::UPDATE_PAUSED;
	mySprite.Position.TileX = 7;
	mySprite.Position.TileY = 5;
	mySprite.Position.SubX = 0;
	mySprite.Position.SubY = 0;
	TileFilenames.push_back("TestSprite.png");
	MainGameState.Graphics.AllSprites.push_back(mySprite);*/

	MainGameState.Graphics.GraphicsRefreshRequired = true;
	MainGameState.MinorTicUpdate = Logic_MinorTic_Battle;
	MainGameState.MajorTicUpdate = Logic_MajorTic_Battle;

	MainGameState.Music.SetBgm("Map1.ogg");
	MainGameState.Music.Play();
	return true;
}

bool Logic_MinorTic_Battle(GameState &MainGameState){
	// Update sprites if needed.
	return true;
}

bool Logic_MajorTic_Battle(GameState &MainGameState){
	unsigned char Buttons = MainGameState.Interface.GetButtonState();
	if(Buttons & InterfaceCore::KEY_EXIT)
		AbortGame(MainGameState);
	else if(Buttons & InterfaceCore::KEY_UP)
		MainGameState.Graphics.AllSprites[0].Position.TileY--;
	else if(Buttons & InterfaceCore::KEY_DOWN)
		MainGameState.Graphics.AllSprites[0].Position.TileY++;
	else if(Buttons & InterfaceCore::KEY_LEFT)
		MainGameState.Graphics.AllSprites[0].Position.TileX--;
	else if(Buttons & InterfaceCore::KEY_RIGHT)
		MainGameState.Graphics.AllSprites[0].Position.TileX++;
	else if(Buttons & InterfaceCore::KEY_OK)
		MainGameState.Music.PushBgm("Attack.ogg");
	else if(Buttons & InterfaceCore::KEY_CANCEL)
		MainGameState.Music.PopBgm();
	if(Buttons & (InterfaceCore::KEY_RIGHT | InterfaceCore::KEY_LEFT | InterfaceCore::KEY_UP | InterfaceCore::KEY_DOWN)){
		MainGameState.Graphics.GraphicsRefreshRequired = true;
		MainGameState.FramesUntilLowRate = 5;
	} else
		MainGameState.FramesUntilLowRate = 0;
	return true;
}
