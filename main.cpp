//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Program Name: Shining Force Community Project Engine
// Author:  Sean Happel
//
#define VERSION_MAJOR      0 // Still in pre-release
#define VERSION_MINOR      2 // Pre-release version 2
#define VERSION_REVISION   0 // First edit of this revision
//
#define BUILD_IS_RELEASED  false // Candidate build
//
// See Changelog.txt for more details
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <SDL/SDL.h>
#include <iostream>
#include <stdexcept>
#include "main.h"
#include "GraphicsCore.h"
#include "InterfaceCore.h"
#include "ResourceCore.h"
#include <vorbis/vorbisfile.h>
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

	// Validate manifest
	if(!InitializeResources("TestMap1.tmx", myGraphicsCore, myGameState)){
		cerr << "Failed to initialize resource pool.  Verify resource files and retry.  Aborting." << endl;
		return -1;
	}

	myGameState.MainGameMode = GameState::MODE_STARTSCREEN;
	myGameState.InitializeNewMode = true;
	myGameState.Graphics.GraphicsFlipRequired = false;
	myGameState.Graphics.GraphicsRefreshRequired = false;
	// Enter master loop
	try{
		std::string NextZone;
		while(myGameState.MainGameMode != GameState::MODE_EXITPROGRAM){
			// Do default initialization
			myGameState.Graphics.Reset();
			myGameState.FramesUntilLowRate = 0; // Force a major update this turn, unless overridden
			// Do further initialization, and enter loop
			switch(myGameState.MainGameMode){
			case GameState::MODE_INTRO:
			case GameState::MODE_STARTSCREEN:
				if(!Initialize_StartScreen(myGraphicsCore,myGameState))
					throw "Failed to initialize new mode.";
				NextZone = MasterLoop<Logic_MajorTic_StartScreen,Logic_MinorTic_StartScreen>(myGraphicsCore,myGameState);
				break;
			case GameState::MODE_WORLDMAP:
			case GameState::MODE_TOWN:
			case GameState::MODE_BATTLE:
				if(!Initialize_Battle(myGraphicsCore,myGameState))
					throw "Failed to initialize new mode.";
				NextZone = MasterLoop<Logic_MajorTic_Battle,Logic_MinorTic_Battle>(myGraphicsCore,myGameState);
				break;
			case GameState::MODE_ENDING:
				throw std::runtime_error("Not implemented yet!");
			}
		}
	}catch(std::exception &e){
		cerr << "std::exception caught: " << e.what() << endl;
	}catch(const char* &e){
		cerr << e << endl << "Aborting." << endl;
	}catch(...){
		cerr << "Unhandled unknown throwable caught.  Aborting." << endl;
	}
	// Close everything.

	return 0;
}

