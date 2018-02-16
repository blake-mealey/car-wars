#pragma once

enum GameState {
	GameState_Menu,
	GameState_Playing,
	GameState_Paused
};

class StateManager {
public:
	static void SetState(GameState state);

	static GameState GetState();

	static bool IsState(GameState state);
private:
	static GameState currentState;
};