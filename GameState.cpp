#include "GameState.h"

bool GameData::Initialize(unsigned int MaxMapSizeX, unsigned int MaxMapSizeY, unsigned int MaxNumMobs, unsigned int MaxNumSpecials){
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
	if(SpecialBufferSize < MaxNumSpecials){
		if(Specials)
			delete[] Specials;
		Specials = new Special[MaxNumSpecials];
		SpecialBufferSize = MaxNumSpecials;
		SpecialsBufEnd = Specials;
	}
	return true;
}
