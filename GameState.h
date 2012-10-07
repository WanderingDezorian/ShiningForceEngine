#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "GraphicsCore.h"
#include "InterfaceCore.h"

struct GameState;

typedef bool (*GameLogicFunction) (GameState &MainGameState);

struct GameState{
	enum TypeMainGameMode{
		MODE_STARTSCREEN,
		MODE_TOWN,
		MODE_BATTLE,
		MODE_ENDING,
		MODE_EXITPROGRAM
	} MainGameMode;
	bool InitializeNewMode;
	int FramesUntilLowRate; // Must be set positive by low-rate logic call.

	GameLogicFunction InitializeFunction;
	GameLogicFunction MinorTicUpdate;
	GameLogicFunction MajorTicUpdate;
	// Here is the interface data
	InterfaceCore Interface;
	// Here is the graphical data
	GraphicalData Graphics;

};

#endif // GAMESTATE_H
