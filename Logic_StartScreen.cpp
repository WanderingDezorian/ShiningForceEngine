#include "main.h"
#include <iostream>
void AbortGame(GameState &MainGameState, const char* Message){
	if(Message)
		std::cerr << Message;
	MainGameState.MainGameMode = GameState::MODE_EXITPROGRAM;
	MainGameState.InitializeFunction = Initialize_ExitProgram;
}

bool Initialize_StartScreen(GameState &MainGameState){
	MainGameState.Graphics.SpecialBuffers.resize(2);
	if(!MainGameState.Graphics.SpecialBuffers[0].Load("TitleFrame1.bmp"))
		return false;
	if(!MainGameState.Graphics.SpecialBuffers[1].Load("TitleFrame2.bmp"))
		return false;
	MainGameState.Graphics.GraphicsRefreshRequired = true;
	MainGameState.Graphics.SpecialBuffers[0].SetEnable(true);
	MainGameState.Graphics.SpecialBuffers[1].SetEnable(false);
	MainGameState.MinorTicUpdate = Logic_MinorTic_StartScreen;
	MainGameState.MajorTicUpdate = Logic_MajorTic_StartScreen;
	return true;
}

bool Logic_MinorTic_StartScreen(GameState &MainGameState){
	unsigned char Buttons = MainGameState.Interface.GetButtonState();
	if(Buttons & InterfaceCore::KEY_EXIT)
		AbortGame(MainGameState);
	else if(Buttons & InterfaceCore::KEY_START)
		AbortGame(MainGameState,"Start button pressed, entering battle.");//MainGameState.InitializeFunction = Initialize_Battle;
	return true;
}

bool Logic_MajorTic_StartScreen(GameState &MainGameState){
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

bool Initialize_ExitProgram(GameState &MainGameState){
	MainGameState.MainGameMode == GameState::MODE_EXITPROGRAM;
	return true;
}
bool Logic_MinorTic_ExitProgram(GameState &MainGameState){
	return true;
}

bool Logic_MajorTic_ExitProgram(GameState &MainGameState){
	MainGameState.FramesUntilLowRate = 30;
	return true;
}

