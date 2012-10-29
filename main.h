#ifndef MAIN_H
#define MAIN_H

#include "GameState.h"

// TODO:  Add an Exit function

void AbortGame(GameState &MainGameState, const char* Message = 0);

bool Initialize_StartScreen(GraphicsCore &GCore, GameState &MainGameState, std::vector<std::string> &TileFilenames);
bool Initialize_Town(GraphicsCore &GCore,GameState &MainGameState, std::vector<std::string> &TileFilenames);
bool Initialize_Battle(GraphicsCore &GCore, GameState &MainGameState, std::vector<std::string> &TileFilenames);
bool Initialize_Ending(GraphicsCore &GCore, GameState &MainGameState, std::vector<std::string> &TileFilenames);
bool Initialize_ExitProgram(GraphicsCore &GCore, GameState &MainGameState, std::vector<std::string> &TileFilenames);

bool Logic_MinorTic_StartScreen(GameState &MainGameState);
bool Logic_MinorTic_Town(GameState &MainGameState);
bool Logic_MinorTic_Battle(GameState &MainGameState);
bool Logic_MinorTic_Ending(GameState &MainGameState);
bool Logic_MinorTic_ExitProgram(GameState &MainGameState);

bool Logic_MajorTic_StartScreen(GameState &MainGameState);
bool Logic_MajorTic_Town(GameState &MainGameState);
bool Logic_MajorTic_Battle(GameState &MainGameState);
bool Logic_MajorTic_Ending(GameState &MainGameState);
bool Logic_MajorTic_ExitProgram(GameState &MainGameState);

#endif // MAIN_H
