#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "GraphicsCore.h"
#include "InterfaceCore.h"
#include "MusicCore.h"
#include <string>

struct GameState;

typedef bool (*GameLogicFunction) (GameState &MainGameState);
typedef bool (*ModeInitFunction) (GraphicsCore &GCore, GameState &MainGameState, std::vector<std::string> &TileFilenames);

struct Mob{ // TODO:  Move to own file, eventually.
	Point OccupiedTile;
	bool Busy; // Talking to hero, etc.  Do not assign additional actions.
	unsigned int Speed; // Movement speed.  = 0 if movement complete.
	Mob() : OccupiedTile(), Busy(false), Speed(0) {}
};

struct GameState{
	enum TypeMainGameMode{
		MODE_STARTSCREEN,
		MODE_TOWN,
		MODE_BATTLE,
		MODE_ENDING,
		MODE_EXITPROGRAM
	} MainGameMode;
	bool InitializeNewMode;
	unsigned int FramesInMode;
	int FramesUntilLowRate; // Must be set positive by low-rate logic call.

	unsigned int SelectedMob;
	std::vector<Mob> Mobs;

	ModeInitFunction InitializeFunction;
	GameLogicFunction MinorTicUpdate;
	GameLogicFunction MajorTicUpdate;
	// Here is the interface data
	InterfaceCore Interface;
	MusicCore Music;
	// Here is the graphical data
	GraphicalData Graphics;

};

#endif // GAMESTATE_H
