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
    case GameState_Menu_Settings:
        ContentManager::DestroySceneAndLoadScene("Menu.json");
        GuiHelper::LoadGuiSceneToCamera(0, "GUIs/OptionsMenu_GUI.json");
        break;
    case GameState_Playing:
        if (previousState < __GameState_Menu_End) {
            Game::Instance().InitializeGame();
        } else if (previousState == GameState_Paused) {
            EntityManager::DestroyEntities(EntityManager::FindEntities("PauseMenu"));
        }
        break;
    case GameState_Paused:
        break;
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