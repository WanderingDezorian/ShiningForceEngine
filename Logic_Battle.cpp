#include "main.h"
#include "ResourceCore.h"

bool Logic_MinorTic_Battle(GameState &MainGameState){
	MainGameState.FramesInMode++;
	Mob* iMob = &MainGameState.Mobs[0];
	Sprite* iSprite = &MainGameState.Graphics.AllSprites[0];
	Point TargetPos = iMob->OccupiedTile * PTileSize;
	if(iMob->Speed){
		iSprite->Position.MoveTowards(TargetPos,iMob->Speed);
		MainGameState.Graphics.MasterCamera.MoveTowards(MainGameState.Graphics.DesiredMasterCamera,iMob->Speed);
		if(iSprite->Position == TargetPos)
			iMob->Speed = 0;
		MainGameState.Graphics.GraphicsRefreshRequired = true;
	}
	// Update sprites if needed.
	if((MainGameState.FramesInMode & 0x03) == 0){
		iSprite->Update();
		MainGameState.Graphics.GraphicsRefreshRequired = true;
	}
	return true;
}

bool Logic_MajorTic_Battle(GameState &MainGameState, std::string &RetVal){
	Mob* SelectedMob = &MainGameState.Mobs[0];
	Sprite* SelectedSprite = &MainGameState.Graphics.AllSprites[0];
	if(SelectedMob->Speed == 0){
		unsigned char Buttons = MainGameState.Interface.PeekButtonState();
		if(Buttons & InterfaceCore::KEY_EXIT)
			AbortGame(MainGameState);
		else if(Buttons & InterfaceCore::KEY_UP){
			SelectedSprite->OrientationBufferOffset = 2;
			if(SelectedMob->OccupiedTile.Y != 0){
				if((MainGameState.Data.Blockers[(SelectedMob->OccupiedTile.Y - 1) * MainGameState.Graphics.MasterMapSizeInTiles.X + SelectedMob->OccupiedTile.X] & GameData::BLOCKER_MASK_DOWN) == 0){
					SelectedMob->OccupiedTile.Y--;
					int Desired = (SelectedMob->OccupiedTile.Y - 2) * PTileSize;
					if((Desired < MainGameState.Graphics.MasterCamera.Y) && (Desired >= 0))
						MainGameState.Graphics.DesiredMasterCamera.Y = Desired;
				}
			}
		}
		else if(Buttons & InterfaceCore::KEY_DOWN){
			SelectedSprite->OrientationBufferOffset = 0;
			if(SelectedMob->OccupiedTile.Y != MainGameState.Graphics.MasterMapSizeInTiles.Y - 1){
				if((MainGameState.Data.Blockers[(SelectedMob->OccupiedTile.Y + 1) * MainGameState.Graphics.MasterMapSizeInTiles.X + SelectedMob->OccupiedTile.X] & GameData::BLOCKER_MASK_UP) == 0){
					SelectedMob->OccupiedTile.Y++;
					int Desired = (int(SelectedMob->OccupiedTile.Y) - 7) * PTileSize;
					if((Desired > int(MainGameState.Graphics.MasterCamera.Y)) && (SelectedMob->OccupiedTile.Y < MainGameState.Graphics.MasterMapSizeInTiles.Y - 2))
						MainGameState.Graphics.DesiredMasterCamera.Y = Desired;
				}
			}
		}
		else if(Buttons & InterfaceCore::KEY_LEFT){
			SelectedSprite->OrientationBufferOffset = 6;
			if(SelectedMob->OccupiedTile.X != 0){
				if((MainGameState.Data.Blockers[SelectedMob->OccupiedTile.Y * MainGameState.Graphics.MasterMapSizeInTiles.X + SelectedMob->OccupiedTile.X - 1] & GameData::BLOCKER_MASK_RIGHT) == 0){
					SelectedMob->OccupiedTile.X--;
					int Desired = (SelectedMob->OccupiedTile.X - 2) * PTileSize;
					if((Desired < MainGameState.Graphics.MasterCamera.X) && (Desired >= 0))
						MainGameState.Graphics.DesiredMasterCamera.X = Desired;
				}
			}
		}
		else if(Buttons & InterfaceCore::KEY_RIGHT){
			SelectedSprite->OrientationBufferOffset = 4;
			if(SelectedMob->OccupiedTile.X != MainGameState.Graphics.MasterMapSizeInTiles.X - 1){
				if((MainGameState.Data.Blockers[SelectedMob->OccupiedTile.Y * MainGameState.Graphics.MasterMapSizeInTiles.X + SelectedMob->OccupiedTile.X + 1] & GameData::BLOCKER_MASK_LEFT) == 0){
					SelectedMob->OccupiedTile.X++;
					int Desired = int(SelectedMob->OccupiedTile.X * PTileSize) + (3*PTileSize - 320);
					if((Desired > int(MainGameState.Graphics.MasterCamera.X)) && (SelectedMob->OccupiedTile.X < MainGameState.Graphics.MasterMapSizeInTiles.X - 2))
						MainGameState.Graphics.DesiredMasterCamera.X = Desired;
				}
			}
		}
		else if(Buttons & InterfaceCore::KEY_OK)
			MainGameState.Music.PushBgm("Attack.ogg");
		else if(Buttons & InterfaceCore::KEY_CANCEL)
			MainGameState.Music.PopBgm();
		if(Buttons & (InterfaceCore::KEY_RIGHT | InterfaceCore::KEY_LEFT | InterfaceCore::KEY_UP | InterfaceCore::KEY_DOWN)){
			SelectedMob->Speed = 4;
			MainGameState.FramesUntilLowRate = 5; // May be set lower if
		}
	} else
		MainGameState.FramesUntilLowRate = 0;
	return true;
}


