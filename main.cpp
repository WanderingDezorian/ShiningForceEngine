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

#include <iostream>
#include "main.h"
using namespace std;

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

	// Enter master loop

	MainGameMode NextModeToEnter = MODE_STARTSCREEN;

	while(NextModeToEnter != MODE_EXITPROGRAM){
		switch(NextModeToEnter){
			case MODE_STARTSCREEN:
			case MODE_TOWN:
			case MODE_BATTLE:
			case MODE_ENDING:
			case MODE_EXITPROGRAM:
				cerr << "Unhandled mode requested.  Exiting." << endl;
				NextModeToEnter = MODE_EXITPROGRAM;
		}
	}

	// Close everything.

	return 0;
}


