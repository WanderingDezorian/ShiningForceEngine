#ifndef MAIN_H
#define MAIN_H

#include "GameState.h"
#include <iostream>

// TODO:  Add an Exit function

void AbortGame(GameState &MainGameState, const char* Message = 0);

// Each of these returns a string containing the next level to travel to.
template<GameMajorLogicFunction MajorTic, GameMinorLogicFunction MinorTic> std::string MasterLoop(GraphicsCore &GCore, GameState &MainGameState);

bool Initialize_StartScreen(GraphicsCore &GCore, GameState &MainGameState);

bool Initialize_Town(GraphicsCore &GCore,GameState &MainGameState);
bool Initialize_Battle(GraphicsCore &GCore, GameState &MainGameState);
bool Initialize_Ending(GraphicsCore &GCore, GameState &MainGameState, std::vector<std::string> &TileFilenames);
bool Initialize_ExitProgram(GraphicsCore &GCore, GameState &MainGameState, std::vector<std::string> &TileFilenames);

bool Logic_MinorTic_StartScreen(GameState &MainGameState);
bool Logic_MinorTic_Town(GameState &MainGameState);
bool Logic_MinorTic_Battle(GameState &MainGameState);
bool Logic_MinorTic_Ending(GameState &MainGameState);
bool Logic_MinorTic_ExitProgram(GameState &MainGameState);

bool Logic_MajorTic_StartScreen(GameState &MainGameState, std::string &NextZone);
bool Logic_MajorTic_Town(GameState &MainGameState, std::string &NextZone);
bool Logic_MajorTic_Battle(GameState &MainGameState, std::string &NextZone);
bool Logic_MajorTic_Ending(GameState &MainGameState, std::string &NextZone);
bool Logic_MajorTic_ExitProgram(GameState &MainGameState, std::string &NextZone);

template<GameMajorLogicFunction MajorTic, GameMinorLogicFunction MinorTic> std::string MasterLoop(GraphicsCore &GCore, GameState &MainGameState){
	// Enter master loop
	const clock_t ClocksPerFrame = CLOCKS_PER_SEC / 30;
	const clock_t ClocksPerStep = ClocksPerFrame / 3;

	std::string RetVal;
	while(1){
		clock_t NextFrame = clock();
		clock_t NextStep = NextFrame + ClocksPerStep;
		NextFrame += ClocksPerFrame;

		MainGameState.Interface.PollInterface(NextStep);
		NextStep += ClocksPerStep;
		// Process game logic
		if(MainGameState.FramesUntilLowRate)
			MainGameState.FramesUntilLowRate--;
		else if(!MajorTic(MainGameState, RetVal))
			return RetVal;
		MinorTic(MainGameState);
		if(clock() >= NextStep)
			std::cerr << "Logic overran alloted time" << std::endl;
		// Wait for next frame
		if(MainGameState.Graphics.GraphicsRefreshRequired){
			GCore.PrepareNextFrame(MainGameState.Graphics);
			MainGameState.Graphics.GraphicsFlipRequired = true;
			MainGameState.Graphics.GraphicsRefreshRequired = false;
		}
		if(clock() >= NextFrame)
			std::cerr << "Graphic redraw overran alloted time" << std::endl;
		while(clock() < NextFrame);
		if(MainGameState.Graphics.GraphicsFlipRequired){
			GCore.FlipBuffer();
			MainGameState.Graphics.GraphicsFlipRequired = false;
		}
	}
}
#endif // MAIN_H
