#include "StateManager.h"


Time StateManager::deltaTime = 0;
Time StateManager::gameTime = 0;

size_t StateManager::menuIndex = 0;

GameState StateManager::currentState = GameState_Menu;

GameState StateManager::GetState() {
	return currentState;
}

void StateManager::SetState(GameState state) {
	currentState = state;
	// TODO: Fire event
}

bool StateManager::IsState(GameState state) {
	return currentState == state;
}

std::string StateManager::GameStateToString() {
	switch (currentState) {
	case 0:
		return "GameState_Menu";
		break;
	case 1:
		return "GameState_Menu_Start";
		break;
	case 2:
		return "GameState_Menu_Settings";
		break;
	case 3:
		return "GameState_Menu_Start_CharacterSelect";
		break;
	case 4:
		return "GameState_Playing";
		break;
	case 5:
		return "GameState_Paused";
		break;
	}
}