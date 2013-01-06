#include "GameState.h"

bool GameData::Initialize(unsigned int MaxMapSizeX, unsigned int MaxMapSizeY, unsigned int MaxNumMobs){
	unsigned int NeededBufferSize = MaxMapSizeX * MaxMapSizeY;
	if(BlockerBufferSize < NeededBufferSize){
		if(Blockers)
			delete[] Blockers;
		Blockers = new unsigned char[NeededBufferSize];
		BlockerBufferSize = NeededBufferSize;
	}
	if(MobBufferSize < MaxNumMobs){
		if(Mobs)
			delete[] Mobs;
		Mobs = new Mob[MaxNumMobs];
		MobBufferSize = MaxNumMobs;
		SelectedMob = Mobs;
	}
	return true;
}
