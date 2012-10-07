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
	GameState myGameState;
	if(SDL_WasInit(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != (SDL_INIT_VIDEO | SDL_INIT_AUDIO)){
		cerr << "Failed to initialize SDL.  Aborting." << endl;
		return -1;
	}

	myGameState.MainGameMode = GameState::MODE_STARTSCREEN;
	myGameState.InitializeNewMode = true;
	myGameState.Graphics.GraphicsFlipRequired = false;
	myGameState.Graphics.GraphicsRefreshRequired = false;
	myGameState.InitializeFunction = Initialize_StartScreen;

	// Enter master loop
	const clock_t ClocksPerFrame = CLOCKS_PER_SEC / 30;

	while(myGameState.MainGameMode != GameState::MODE_EXITPROGRAM){
		clock_t NextFrame = clock() + ClocksPerFrame;
		if(myGameState.Graphics.GraphicsFlipRequired){
			myGraphicsCore.FlipBuffer();
			myGameState.Graphics.GraphicsFlipRequired = false;
		}
		if(myGameState.Graphics.GraphicsRefreshRequired){
			myGraphicsCore.PrepareNextFrame(myGameState.Graphics);
			myGameState.Graphics.GraphicsFlipRequired = true;
			myGameState.Graphics.GraphicsRefreshRequired = false;
		}
		myGameState.Interface.PollInterface(NextFrame);
		// Process game logic
		if(myGameState.InitializeFunction){
			myGameState.Graphics.Reset();
			myGameState.FramesUntilLowRate = 0; // Force a major update this turn, unless overridden
			std::vector<std::string> TileFilenames;
			if(myGameState.InitializeFunction(myGameState,TileFilenames))
				myGameState.InitializeFunction = 0;
			else
				AbortGame(myGameState,"Failed to initialize new mode.  Aborting program.");
			if(!myGraphicsCore.LoadTileBuffer(TileFilenames))
				AbortGame(myGameState,"Failed to load required tiles.  Aborting program.");
		} // if(myGameState.InitializeNewMode)
		myGameState.MinorTicUpdate(myGameState);
		if(myGameState.FramesUntilLowRate)
			myGameState.FramesUntilLowRate--;
		else{
			myGameState.MajorTicUpdate(myGameState);
			if(!myGameState.FramesUntilLowRate) // TODO:  Should be assert
				AbortGame(myGameState,"Failed to update FramesUntilLowRate.  Aborting program.");
		}
		// Wait for next frame
		while(clock() < NextFrame);
	}

	// Close everything.

	return 0;
}


