#ifndef GAMESTATE_H
#define GAMESTATE_H

struct GameState{
	enum TypeMainGameMode{
		MODE_STARTSCREEN,
		MODE_TOWN,
		MODE_BATTLE,
		MODE_ENDING,
		MODE_EXITPROGRAM
	} MainGameMode;
	bool InitializeNewMode;
	int FramesUntilLowRate; // Must be set positive by low-rate logic call.

	bool GraphicsFlipRequired;
	bool GraphicsBufferRefreshRequred;

};

#endif // GAMESTATE_H
