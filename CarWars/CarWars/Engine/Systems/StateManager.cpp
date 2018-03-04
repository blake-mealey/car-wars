#include "StateManager.h"
#include "GuiHelper.h"
#include "Content/ContentManager.h"
#include "../Entities/EntityManager.h"
#include "InputManager.h"
#include "Game.h"


Time StateManager::deltaTime = 0;
Time StateManager::gameTime = 0;

GameState StateManager::currentState = GameState_Menu;

GameState StateManager::GetState() {
	return currentState;
}

void StateManager::SetState(GameState state) {
    GameState previousState = currentState;
	currentState = state;

    switch (currentState) {
    case GameState_Exit:
        glfwSetWindowShouldClose(Graphics::Instance().GetWindow(), true);
        break;
    case GameState_Menu:
        ContentManager::DestroySceneAndLoadScene("MainMenu.json");
        GuiHelper::LoadGuiSceneToCamera(0, "GUIs/MainMenu_GUI.json");
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
    case GameState_Playing:
        if (previousState < __GameState_Menu_End) {
            Game::Instance().InitializeGame();
        }
    }

    // TODO: Fire event

    std::cout << "Game State: " << GameStateToString() << std::endl;
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
    default:
        return "";
	}
}