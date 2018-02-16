#pragma once

#include "../Systems/Time.h"

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

	static Time deltaTime;
	static Time gameTime;
private:
	static GameState currentState;
};