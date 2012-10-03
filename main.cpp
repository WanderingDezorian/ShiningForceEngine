//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Program Name: Shining Force Community Project Engine
// Author:  Sean Happel
//
#define VERSION_MAJOR      0 // Still in pre-release
#define VERSION_MINOR      1 // Pre-release version 1
#define VERSION_REVISION   0 // First edit of this revision
//
#define BUILD_IS_RELEASED  false // Candidate build
//
// See Changelog.txt for more details
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//TODO: Game has simple start-screen
//TODO: Game has simple map (checkerboard tiles, no scrolling, win by leaving map)
//TODO: Demonstrate graphics works
//TODO: Demonstrate music works
//TODO: Demonstrate keyboard control works

#include <SDL/SDL.h>
#include <iostream>
#include "main.h"
#include "GraphicsCore.h"
#include "InterfaceCore.h"
#include "GameState.h"
using namespace std;

// Main thread master loop-
//  Flip buffer
//  Update graphics
//  Update inputs
//  High-rate data
//  Opt low-rate data

// Second thread-
//   AI.

void PrintHelpMessage(){
	cout << "Name:" << endl
		 << "   Shining Force Community Project Engine" << endl
		 << "      Version " << VERSION_MAJOR << '.' << VERSION_MINOR << '.' << VERSION_REVISION;
	if(!BUILD_IS_RELEASED)
		cout << " (CANDIDATE BUILD)";
	cout << endl << endl
		 << "Synopsis:" << endl
		 << "   ShiningForceEngine [no-args]" << endl << endl
		 << "Description:" << endl
		 << "   TBD"
		 << endl;
}

int main(int argc, char** argv){
	PrintHelpMessage();

	// Initialize all systems
	GraphicsCore myGraphicsCore; // Must be first - initializes SDL
	InterfaceCore myInterfaceCore;
	GameState myGameState;
	if(SDL_WasInit(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != (SDL_INIT_VIDEO | SDL_INIT_AUDIO)){
		cerr << "Failed to initialize SDL.  Aborting." << endl;
		return -1;
	}

	myGameState.MainGameMode = GameState::MODE_STARTSCREEN;
	myGameState.InitializeNewMode = true;
	myGameState.GraphicsFlipRequired = false;
	myGameState.GraphicsBufferRefreshRequred = false;

	// Enter master loop
	const clock_t ClocksPerFrame = CLOCKS_PER_SEC / 30;

	while(myGameState.MainGameMode != GameState::MODE_EXITPROGRAM){
		clock_t NextFrame = clock() + ClocksPerFrame;
		if(myGameState.GraphicsFlipRequired){
			myGraphicsCore.FlipBuffer();
			myGameState.GraphicsFlipRequired = false;
		}
		if(myGameState.GraphicsBufferRefreshRequred){
			myGraphicsCore.PrepareNextFrame(myGameState);
			myGameState.GraphicsBufferRefreshRequred = false;
		}
		myInterfaceCore.PollInterface(NextFrame);
		// Process game logic
		switch(myGameState.MainGameMode){
			case GameState::MODE_STARTSCREEN:
				if(myGameState.InitializeNewMode){
					SDL_Surface* File = SDL_LoadBMP("TitleFrame1.bmp");
					if(File == NULL){
						cerr << "Unable to load file TitleFrame1.bmp" << endl;
						myGameState.MainGameMode = GameState::MODE_EXITPROGRAM;
						break;
					}
					SDL_BlitSurface(File,NULL,myGraphicsCore.GetMainWindow(),NULL);
//					myGraphicsCore.FlipBuffer();
//					File = SDL_LoadBMP("TitleFrame2.bmp");
//					if(File == NULL){
//						cerr << "Unable to load file TitleFrame2.bmp" << endl;
//						myGameState.MainGameMode = GameState::MODE_EXITPROGRAM;
//						break;
//					}
//					SDL_BlitSurface(File,NULL,myGraphicsCore.GetMainWindow(),NULL);
					myGameState.FramesUntilLowRate = 30;
					myGameState.InitializeNewMode = false;
				}
				if(myGameState.FramesUntilLowRate)
					myGameState.FramesUntilLowRate--;
				else{
					myGameState.GraphicsFlipRequired = true;
					myGameState.FramesUntilLowRate = 30;
				}
			case GameState::MODE_TOWN:
			case GameState::MODE_BATTLE:
			case GameState::MODE_ENDING:
				if(myInterfaceCore.GetButtonState() & InterfaceCore::KEY_EXIT)
					myGameState.MainGameMode = GameState::MODE_EXITPROGRAM;
			case GameState::MODE_EXITPROGRAM:
				break;
			default:
				cerr << "Unhandled mode requested.  Exiting." << endl;
				myGameState.MainGameMode = GameState::MODE_EXITPROGRAM;
		}
	}

	// Close everything.

	return 0;
}


