#include "main.h"
#include <iostream>
void AbortGame(GameState &MainGameState, const char* Message){
	if(Message)
		throw(Message);
	MainGameState.MainGameMode = GameState::MODE_EXITPROGRAM;
	throw("Game abort requested.");
}

#include "ResourceCore.h"
bool Initialize_StartScreen(GraphicsCore &GCore, GameState &MainGameState){
	MainGameState.Graphics.SpecialBuffers.resize(2);
	if(!MainGameState.Graphics.SpecialBuffers[0].Load("TitleFrame1.bmp"))
		return false;
	if(!MainGameState.Graphics.SpecialBuffers[1].Load("TitleFrame2.bmp"))
		return false;
	MainGameState.Graphics.GraphicsRefreshRequired = true;
	MainGameState.Graphics.SpecialBuffers[0].SetEnable(true);
	MainGameState.Graphics.SpecialBuffers[1].SetEnable(false);
	MainGameState.Music.SetBgm("Opening.ogg");
	MainGameState.Music.Play();
	return true;
}

bool Logic_MinorTic_StartScreen(GameState &MainGameState){
	unsigned char Buttons = MainGameState.Interface.GetButtonState();
	if(Buttons & InterfaceCore::KEY_EXIT)
		AbortGame(MainGameState);
	else if(Buttons & InterfaceCore::KEY_START){
		std::cerr << "Start button pressed, entering battle." << std::endl;
		MainGameState.MainGameMode = GameState::MODE_BATTLE;
		MainGameState.FramesUntilLowRate = 0; // Force a low-rate update.
	}
	return true;
}

bool Logic_MajorTic_StartScreen(GameState &MainGameState, std::string &NextZone){
	if(MainGameState.MainGameMode != GameState::MODE_STARTSCREEN){
		NextZone = "%start";
		return false;
	}
	if(MainGameState.Graphics.SpecialBuffers[0].GetEnable()){
		MainGameState.Graphics.SpecialBuffers[0].SetEnable(false);
		MainGameState.Graphics.SpecialBuffers[1].SetEnable(true);
	} else {
		MainGameState.Graphics.SpecialBuffers[0].SetEnable(true);
		MainGameState.Graphics.SpecialBuffers[1].SetEnable(false);
	}
	MainGameState.Graphics.GraphicsRefreshRequired = true;
	MainGameState.FramesUntilLowRate = 15;
	return true;
}

bool Initialize_ExitProgram(GraphicsCore &GCore, GameState &MainGameState, std::vector<std::string> &TileFilenames){
	MainGameState.MainGameMode = GameState::MODE_EXITPROGRAM;
	return true;
}
bool Logic_MinorTic_ExitProgram(GameState &MainGameState){
	return false;
}

bool Logic_MajorTic_ExitProgram(GameState &MainGameState, std::string &NextZone){
	MainGameState.FramesUntilLowRate = 0;
	NextZone = "%void";
	return false;
}

