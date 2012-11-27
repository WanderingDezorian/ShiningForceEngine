#include "GameState.h"

bool GameData::Initialize(unsigned int MaxMapSizeX, unsigned int MaxMapSizeY, unsigned int MaxNumMobs){
	unsigned int NeededBufferSize = MaxMapSizeX * MaxMapSizeY;
	if(BlockerBufferSize < NeededBufferSize){
		if(Blockers)
			delete[] Blockers;
		Blockers = new unsigned char[NeededBufferSize];
		BlockerBufferSize = NeededBufferSize;
	}
	if(MobBufferSize < NeededBufferSize){
		if(Mobs)
			delete[] Mobs;
		Mobs = new Mob[NeededBufferSize];
		MobBufferSize = NeededBufferSize;
		SelectedMob = Mobs;
	}
	return true;
}
