#include "StateManager.h"
#include "../Components/GuiComponents/GuiHelper.h"
#include "Content/ContentManager.h"
#include "../Entities/EntityManager.h"
#include "InputManager.h"
#include "Game.h"


Time StateManager::deltaTime = 0.0;
Time StateManager::gameTime = 0.0;
Time StateManager::globalTime = 0.0;

GameState StateManager::currentState = GameState_Menu;

GameState StateManager::GetState() {
	return currentState;
}

void StateManager::SetState(GameState state) {
    GameState previousState = currentState;
	currentState = state;
	int tie = -1;
    TeamData winner;
	winner.killCount = -1;
    switch (currentState) {
    case GameState_Exit:
        glfwSetWindowShouldClose(Graphics::Instance().GetWindow(), true);
        break;
    case GameState_Menu:
        ContentManager::DestroySceneAndLoadScene("MainMenu.json");
        GuiHelper::LoadGuiSceneToCamera(0, "GUIs/MainMenu_GUI.json");
        break;
	case GameState_Menu_GameEnd:
		ContentManager::DestroySceneAndLoadScene("Menu.json");
		GuiHelper::LoadGuiSceneToCamera(0, "GUIs/GameEnd_GUI.json");
	
		for (TeamData& team : Game::gameData.teams) {
			if (team.killCount > winner.killCount) winner = team;
		}
		for (TeamData& team : Game::gameData.teams) {
			if (team.killCount == winner.killCount && team.killCount > 0) tie ++;
		}
		if (winner.killCount <= 0) GuiHelper::SetFirstGuiText("WinnerTitle", "game over");
		else if (tie) GuiHelper::SetFirstGuiText("WinnerTitle", "tie");
        else GuiHelper::SetFirstGuiText("WinnerTitle", winner.name + " won!");
		break;

    case GameState_Menu_Start:
        ContentManager::DestroySceneAndLoadScene("Menu.json");
        GuiHelper::LoadGuiSceneToCamera(0, "GUIs/StartMenu_GUI.json");
        break;
    case GameState_Menu_Start_CharacterSelect:
        ContentManager::DestroySceneAndLoadScene("CharacterSelectMenu.json");
        for (size_t i = 0; i < 4; ++i) {
            GuiHelper::LoadGuiSceneToCamera(i, "GUIs/CharacterSelectMenu_GUI.json");      // Load gui to new cameras
        }
        break;
    case GameState_Menu_Settings:
        ContentManager::DestroySceneAndLoadScene("Menu.json");
        GuiHelper::LoadGuiSceneToCamera(0, "GUIs/OptionsMenu_GUI.json");
        break;
    case GameState_Playing:
        if (previousState < __GameState_Menu_End) {
            Game::Instance().InitializeGame();}
        break;
    case GameState_Paused:
        break;
    }

    // TODO: Fire event
}

bool StateManager::IsState(GameState state) {
	return currentState == state;
}

std::string StateManager::GameStateToString() {
	switch (currentState) {
	case 0:
		return "GameState_Menu";
	case 1:
		return "GameState_Menu_Start";
	case 2:
		return "GameState_Menu_Settings";
	case 3:
		return "GameState_Menu_Start_CharacterSelect";
	case 4:
		return "GameState_Playing";
	case 5:
		return "GameState_Paused";
    default:
        return "";
	}
}