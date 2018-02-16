#include "StateManager.h"


Time StateManager::deltaTime = 0;
Time StateManager::gameTime = 0;

GameState StateManager::currentState = GameState_Playing;

void StateManager::SetState(GameState state) {
	currentState = state;
	// TODO: Fire event
}

GameState StateManager::GetState() {
	return currentState;
}

bool StateManager::IsState(GameState state) {
	return currentState == state;
}
