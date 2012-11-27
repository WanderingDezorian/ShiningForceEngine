#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "GraphicsCore.h"
#include "InterfaceCore.h"
#include "MusicCore.h"
#include <string>

// Blocks:  TileBlock, MobBlock, MovementBlock

struct GameState;

typedef bool (*GameLogicFunction) (GameState &MainGameState);
typedef bool (*ModeInitFunction) (GraphicsCore &GCore, GameState &MainGameState, std::vector<std::string> &TileFilenames);

struct Mob{ // TODO:  Move to own file, eventually.
	Point OccupiedTile;
	bool Busy; // Talking to hero, etc.  Do not assign additional actions.
	unsigned int Speed; // Movement speed.  = 0 if movement complete.
	Mob() : OccupiedTile(), Busy(false), Speed(0) {}
};

struct GameData{
	enum BlockerFlags{
		BLOCKER_MAP = 0x01, // Tile is set as a blocking tile
		BLOCKER_MOB = 0x02, // A MOB is occupying the tile
		BLOCKER_MOVEGUARD = 0x04, // Used during battle to limit character's movement
		BLOCKER_BORDER_UP = 0x08,
		BLOCKER_BORDER_DOWN = 0x10,
		BLOCKER_BORDER_LEFT = 0x20,
		BLOCKER_BORDER_RIGHT = 0x40,
		BLOCKER_SPECIAL = 0x80, // Indicates something unusual should happen if character walks here.
		BLOCKER_MASK_UP = 0x0F,
		BLOCKER_MASK_DOWN = 0x17,
		BLOCKER_MASK_LEFT = 0x27,
		BLOCKER_MASK_RIGHT = 0x47,
	};
	unsigned char* Blockers;
	unsigned int BlockerBufferSize;
	Mob* Mobs;
	unsigned int MobBufferSize;
	Mob* SelectedMob;

	GameData(): Blockers(0), BlockerBufferSize(0), Mobs(0), MobBufferSize(0), SelectedMob(0) {}
	~GameData(){ if(Blockers) delete[] Blockers; if(Mobs) delete[] Mobs; }
	bool Initialize(unsigned int MaxMapSizeX, unsigned int MaxMapSizeY, unsigned int MaxNumMobs);
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
	GameData Data;
};

#endif // GAMESTATE_H
