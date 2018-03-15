#include "InputManager.h"

#include "Graphics.h"
#include "Game.h"
#include "../Systems/StateManager.h"
#include "../Entities/EntityManager.h"
#include "../Components/CameraComponent.h"
#include "../Components/RigidbodyComponents/VehicleComponent.h"

#include "PxPhysicsAPI.h"

#include "../Components/WeaponComponents/RailGunComponent.h"
#include "../Components/GuiComponents/GuiComponent.h"
#include "../Components/GuiComponents/GuiHelper.h"
#include "Content/ContentManager.h"

#include "../Systems/Physics/CollisionGroups.h"
#include "../Systems/Physics/RaycastGroups.h"


vector<XboxController*> InputManager::xboxControllers;

Time dt;

InputManager &InputManager::Instance() {
	for (int i = 0; i < XUSER_MAX_COUNT; i++) {
		xboxControllers.push_back(new XboxController(i + 1));
	}

	static InputManager instance;
	return instance;
}

void InputManager::Update() {
	dt = StateManager::deltaTime;
	HandleMouse();
	HandleKeyboard();
	HandleController();
}

void UpdateCamera(Entity *vehicle, CameraComponent *camera, glm::vec2 angleDiffs) {
	float cameraHor = camera->GetCameraHorizontalAngle();
	float cameraVer = camera->GetCameraVerticalAngle();
	float cameraSpd = camera->GetCameraSpeed();
	float cameraNewHor = (cameraHor + (angleDiffs.x * cameraSpd * StateManager::deltaTime.GetSeconds()));
	float cameraNewVer = (cameraVer + (angleDiffs.y * cameraSpd * StateManager::deltaTime.GetSeconds()));

	cameraNewVer = glm::clamp(cameraNewVer, 0.1f, (float) M_PI - 0.1f);
	if (cameraNewHor > M_PI) cameraNewHor -= M_PI * 2;
	if (cameraNewHor < -M_PI) cameraNewHor += M_PI * 2;

	camera->UpdateCameraPosition(vehicle, cameraNewHor, cameraNewVer);
}

void InputManager::HandleMouse() {
	//Mouse Inputs
	//Get Graphics Instance
	Graphics& graphicsInstance = Graphics::Instance();

	if (StateManager::GetState() == GameState_Playing) {
		PlayerData& player = Game::players[0];
		VehicleComponent* vehicle = player.vehicleEntity->GetComponent<VehicleComponent>();
		WeaponComponent* weapon = player.vehicleEntity->GetComponent<WeaponComponent>();
		CameraComponent* cameraC = player.camera;
		//Shoot Weapon
		float rayLength = 100.0f;
		if (Mouse::ButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
			weapon->Charge();
		} else if (Mouse::ButtonDown(GLFW_MOUSE_BUTTON_LEFT)) {
			PxQueryFilterData filterData;
			filterData.data.word0 = RaycastGroups::GetGroupsMask(vehicle->GetRaycastGroup());
			glm::vec3 cameraHit = cameraC->CastRay(rayLength, filterData);
			weapon->Shoot(cameraHit);
		}

		if (Mouse::ButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE)) {
			player.follow = !player.follow;
		}

		//Cursor Inputs
		glfwSetInputMode(graphicsInstance.GetWindow(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		glm::vec2 windowSize = graphicsInstance.GetWindowSize();
		double xPos, yPos;
		Mouse::GetCursorPosition(graphicsInstance.GetWindow(), &xPos, &yPos);

		//Get Camera Component

		glm::vec2 angleDiffs = 10.f * (windowSize*0.5f - glm::vec2(xPos, yPos)) / windowSize;
		angleDiffs.x = -angleDiffs.x;
		UpdateCamera(player.vehicleEntity, cameraC, angleDiffs);

		//Set Cursor to Middle
		glfwSetCursorPos(graphicsInstance.GetWindow(), windowSize.x / 2, windowSize.y / 2);
	} else {
		//Set Cursor Visible
		glfwSetInputMode(graphicsInstance.GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void UpdateVehicleStats(const int playerIndex) {
    PlayerData& player = Game::players[playerIndex];
    Entity* stats = EntityManager::FindEntities("CharacterMenu_Stats")[playerIndex];
    EntityManager::DestroyChildren(stats);
    for (size_t i = 0; i < VehicleType::STAT_COUNT; ++i) {
        Entity* stat = ContentManager::LoadEntity("Menu/CharacterMenuStat.json", stats);
        GuiHelper::SetGuiPositions(stat, glm::vec3(-25.f, 40.f + i*20.f, 0.f));
        GuiHelper::SetFirstGuiText(stat, VehicleType::statDisplayNames[i]);
        GuiHelper::SetSecondGuiText(stat, VehicleType::statValues[player.vehicleType][i]);
    }
}

void UpdateWeaponStats(const int playerIndex) {
    PlayerData& player = Game::players[playerIndex];
    Entity* stats = EntityManager::FindEntities("CharacterMenu_Stats")[playerIndex];
    EntityManager::DestroyChildren(stats);
    for (size_t i = 0; i < WeaponType::STAT_COUNT; ++i) {
        Entity* stat = ContentManager::LoadEntity("Menu/CharacterMenuStat.json", stats);
        GuiHelper::SetGuiPositions(stat, glm::vec3(-25.f, 40.f + i*20.f, 0.f));
        GuiHelper::SetFirstGuiText(stat, WeaponType::statDisplayNames[i]);
        GuiHelper::SetSecondGuiText(stat, WeaponType::statValues[player.weaponType][i]);
    }
}

void CreateStartMenuOption(Entity* container, std::string optionName, std::string optionValue, float index, float count) {
    Entity* option = ContentManager::LoadEntity("Menu/MenuOption.json", container);
    GuiHelper::AddGuiPositions(option, glm::vec3(0.f, 70.f * (index - (count*0.5f)), 0.f));
    GuiHelper::SetFirstGuiText(option ,optionName);
    GuiHelper::SetSecondGuiText(option, optionValue);
    if (index == 0) GuiHelper::SetGuisSelected(option, true);
}

void CreateStartMenu() {
    Entity* optionsContainer = EntityManager::FindEntities("StartMenu_Options")[0];
    const size_t optionsCount = 6;
    CreateStartMenuOption(optionsContainer, "number of lives", to_string(Game::gameData.numberOfLives), 0, optionsCount);
    CreateStartMenuOption(optionsContainer, "number of ai", to_string(Game::gameData.aiCount), 1, optionsCount);
    CreateStartMenuOption(optionsContainer, "map", MapType::displayNames[Game::gameData.map], 2, optionsCount);
    CreateStartMenuOption(optionsContainer, "time limit", to_string(Game::gameData.timeLimitMinutes) + " minutes", 3, optionsCount);
    CreateStartMenuOption(optionsContainer, "kill limit", to_string(Game::gameData.killLimit), 4, optionsCount);
    CreateStartMenuOption(optionsContainer, "game mode", GameModeType::displayNames[Game::gameData.gameMode], 5, optionsCount);
}

void NextNumberOption(Entity* option, int dir, size_t &value, size_t min, size_t max, std::string unit = "") {
    if (!dir) return;
    dir = dir == 0 ? 0 : dir < 0 ? -1 : 1;
    if (value == min && dir < 0) value = max;
    else if (value == max && dir > 0) value = min;
    else value += dir;
    GuiHelper::SetSecondGuiText(option, to_string(value) + unit);

    GuiComponent* arrow = dir < 0 ? GuiHelper::GetThirdGui(option) : GuiHelper::GetFourthGui(option);
    GuiHelper::OpacityEffect(arrow, 0.2, 0.5f, 0.1, 0.1);
}

void NextEnumOption(Entity* option, int dir, size_t &value, size_t count, const std::string* displayNames) {
    if (!dir) return;
    dir = dir == 0 ? 0 : dir < 0 ? -1 : 1;
    if (value == 0 && dir < 0) value = count - 1;
    else if (value == count - 1 && dir > 0) value = 0;
    else value += dir;
    GuiHelper::SetSecondGuiText(option, displayNames[value]);

    GuiComponent* arrow = dir < 0 ? GuiHelper::GetThirdGui(option) : GuiHelper::GetFourthGui(option);
    GuiHelper::OpacityEffect(arrow, 0.2, 0.5f, 0.1, 0.1);
}

void UpdateLeaderboardMenu(Entity* leaderboard, int playerIndex) {
	Entity* container = EntityManager::FindFirstChild(leaderboard, "LeaderboardRows");
	std::vector<VehicleData> allVehicles;
	
	for (size_t i = 0; i < Game::gameData.playerCount; ++i) {
		allVehicles.push_back(Game::players[i]);
	}

	for (AiData& ai : Game::ais) {
		allVehicles.push_back(ai);
	}

	std::sort(allVehicles.begin(), allVehicles.end());

	PlayerData& thisPlayer = Game::players[playerIndex];
	for (size_t i = 0; i < allVehicles.size(); ++i) {
		VehicleData& vehicle = allVehicles[i];
		if (vehicle.id != thisPlayer.id) continue;

		if (i > 9) allVehicles[9] = vehicle;
		break;
	}

	std::vector<Entity*> rows = EntityManager::GetChildren(container);
	for (size_t i = 0; i < rows.size(); ++i) {
		VehicleData& vehicle = allVehicles[i];
		Entity* row = rows[i];
		GuiHelper::SetFirstGuiText(row, vehicle.name);
		GuiHelper::SetSecondGuiText(row, to_string(vehicle.killCount));
	}
}

void CreateLeaderboardMenu(Entity* leaderboard, int playerIndex) {
	Entity* container = EntityManager::FindFirstChild(leaderboard, "LeaderboardRows");

	size_t count = Game::gameData.playerCount + Game::gameData.aiCount;
	count = count < 10 ? count : 10;
	for (size_t i = 0; i < count; ++i) {
		Entity* row = ContentManager::LoadEntity("Menu/LeaderboardRow.json", container);
		GuiHelper::SetGuiPositions(row, glm::vec3(0.f, -85.f + (i * 30.f), 0.f));
	}

	UpdateLeaderboardMenu(leaderboard, playerIndex);
}

void InputManager::NavigateGuis(GuiNavData navData) {
    // If there was no navigation, do nothing
    if (!navData.Valid()) return;

	// Normalize directional inputs
	navData.NormalizeInputs();

    // Get the player for the current controller
    PlayerData& player = Game::players[navData.playerIndex];

	const GameState gameState = StateManager::GetState();
	// Navigate buttons up/down
	if (navData.vertDir) {
		std::string buttonGroupName;
		bool noNavigation = false;
        bool entityNavigation = false;
		switch (gameState) {
		case GameState_Menu:
			buttonGroupName = "MainMenu_Buttons";
			break;
		case GameState_Menu_Start:
			buttonGroupName = "StartMenu_Options";
            entityNavigation = true;
			break;
		case GameState_Menu_Settings:
			buttonGroupName = "OptionsMenu_Buttons";
			break;
        case GameState_Paused:
            buttonGroupName = "PauseMenuButtons";
            break;
		default:
			noNavigation = true;
		}

		if (!noNavigation) {
			Entity* buttonGroup = EntityManager::FindEntities(buttonGroupName)[0];
            if (!entityNavigation) {
                GuiHelper::SelectNextGui(buttonGroup, -navData.vertDir);
            } else {
                GuiHelper::SelectNextEntity(buttonGroup, -navData.vertDir);
            }
		}
	}

	if (navData.horizDir) {
        if (gameState == GameState_Menu_Start) {
            Entity* selected = GuiHelper::GetSelectedEntity("StartMenu_Options");
            if (GuiHelper::FirstGuiContainsText(selected, "lives")) {
                NextNumberOption(selected, navData.horizDir, Game::gameData.numberOfLives, GameData::MIN_NUMBER_OF_LIVES, GameData::MAX_NUMBER_OF_LIVES);
            } else if (GuiHelper::FirstGuiContainsText(selected, "ai")) {
                NextNumberOption(selected, navData.horizDir, Game::gameData.aiCount, GameData::MIN_AI_COUNT, GameData::MAX_AI_COUNT);
            } else if (GuiHelper::FirstGuiContainsText(selected, "map")) {
                NextEnumOption(selected, navData.horizDir, Game::gameData.map, MapType::Count, MapType::displayNames);
            } else if (GuiHelper::FirstGuiContainsText(selected, "time")) {
                NextNumberOption(selected, navData.horizDir, Game::gameData.timeLimitMinutes, GameData::MIN_TIME_LIMIT_MINUTES, GameData::MAX_TIME_LIMIT_MINUTES, " minutes");
            } else if (GuiHelper::FirstGuiContainsText(selected, "kill")) {
                NextNumberOption(selected, navData.horizDir, Game::gameData.killLimit, GameData::MIN_KILL_LIMIT, GameData::MAX_KILL_LIMIT);
            } else if (GuiHelper::FirstGuiContainsText(selected, "game")) {
                NextEnumOption(selected, navData.horizDir, Game::gameData.gameMode, GameModeType::Count, GameModeType::displayNames);
            }
        } else if (gameState == GameState_Menu_Start_CharacterSelect) {
            if (GuiHelper::FirstGuiContainsText("CharacterMenu_Title", "vehicle", navData.playerIndex)) {
                player.vehicleType = (player.vehicleType + navData.horizDir) % VehicleType::Count;
                if (player.vehicleType < 0) player.vehicleType += VehicleType::Count;
                const string text = VehicleType::displayNames[player.vehicleType];
                GuiHelper::SetFirstGuiText("CharacterMenu_SubTitle", text, navData.playerIndex);

                GuiComponent* arrow = navData.horizDir < 0 ? GuiHelper::GetFirstGui("CharacterMenu_Arrows", navData.playerIndex) : GuiHelper::GetSecondGui("CharacterMenu_Arrows", navData.playerIndex);
                GuiHelper::OpacityEffect(arrow, 0.2, 0.5f, 0.1, 0.1);

                Entity* vehicleBox = EntityManager::FindEntities("VehicleBox")[navData.playerIndex];
                EntityManager::DestroyEntity(EntityManager::FindFirstChild(vehicleBox, "Vehicle"));
                ContentManager::LoadEntity(VehicleType::prefabPaths[player.vehicleType], vehicleBox);

                UpdateVehicleStats(navData.playerIndex);
            } else if (GuiHelper::FirstGuiContainsText("CharacterMenu_Title", "weapon", navData.playerIndex)) {
				player.weaponType = (player.weaponType + navData.horizDir) % WeaponType::Count;
				if (player.weaponType < 0) player.weaponType += WeaponType::Count;
				const string text = WeaponType::displayNames[player.weaponType];
				GuiHelper::SetFirstGuiText("CharacterMenu_SubTitle", text, navData.playerIndex);

                GuiComponent* arrow = navData.horizDir < 0 ? GuiHelper::GetFirstGui("CharacterMenu_Arrows", navData.playerIndex) : GuiHelper::GetSecondGui("CharacterMenu_Arrows", navData.playerIndex);
                GuiHelper::OpacityEffect(arrow, 0.2, 0.5f, 0.1, 0.1);

                Entity* vehicleBox = EntityManager::FindEntities("VehicleBox")[navData.playerIndex];
                Entity* vehicle = EntityManager::FindFirstChild(vehicleBox, "Vehicle");
                EntityManager::DestroyEntity(EntityManager::FindFirstChild(vehicle, "GunTurret"));
                Entity* weapon = ContentManager::LoadEntity(WeaponType::turretPrefabPaths[player.weaponType], vehicle);
                weapon->transform.SetPosition(EntityManager::FindFirstChild(vehicle, "GunTurretBase")->transform.GetLocalPosition());
				
                UpdateWeaponStats(navData.playerIndex);
			}
		}
	}

	if (navData.enter) {
		if (gameState == GameState_Menu) {
			GuiComponent *selected = GuiHelper::GetSelectedGui("MainMenu_Buttons");
			if (selected->HasText("start")) {
				StateManager::SetState(GameState_Menu_Start);
                CreateStartMenu();
			} else if (selected->HasText("options")) {
				StateManager::SetState(GameState_Menu_Settings);
			} else if (selected->HasText("exit")) {
				StateManager::SetState(GameState_Exit);
			}
		}
		else if (gameState == GameState_Menu_Settings) {
			GuiComponent *selected = GuiHelper::GetSelectedGui("OptionsMenu_Buttons");
			if (selected->HasText("back")) {
				StateManager::SetState(GameState_Menu);
			}
		}
		else if (gameState == GameState_Menu_Start) {
			Entity* selected = GuiHelper::GetSelectedEntity("StartMenu_Options");
			if (GuiHelper::FirstGuiContainsText(selected, "back")) {
				StateManager::SetState(GameState_Menu);
			} else {
				StateManager::SetState(GameState_Menu_Start_CharacterSelect);
			}
		}
		else if (gameState == GameState_Menu_Start_CharacterSelect) {
			GuiComponent *selected = GuiHelper::GetSelectedGui("CharacterMenu_Buttons", navData.playerIndex);
			if (selected->ContainsText("join")) {
                Game::gameData.playerCount++;

				GuiHelper::SetGuisEnabled("CharacterMenu_Arrows", true, navData.playerIndex);
				GuiHelper::SetGuisEnabled("CharacterMenu_Stats", true, navData.playerIndex);
				GuiHelper::SetFirstGuiText("CharacterMenu_Title", "vehicle selection", navData.playerIndex);
				GuiHelper::SetFirstGuiText("CharacterMenu_SubTitle", VehicleType::displayNames[player.vehicleType], navData.playerIndex);

                Entity* vehicleBox = EntityManager::FindEntities("VehicleBox")[navData.playerIndex];
                ContentManager::LoadEntity(VehicleType::prefabPaths[player.vehicleType], vehicleBox);

                UpdateVehicleStats(navData.playerIndex);

				selected->SetText("a to continue");
			} else {
                if (GuiHelper::FirstGuiContainsText("CharacterMenu_Title", "vehicle", navData.playerIndex)) {
                    GuiHelper::SetFirstGuiText("CharacterMenu_Title", "weapon selection", navData.playerIndex);
                    GuiHelper::SetFirstGuiText("CharacterMenu_SubTitle", WeaponType::displayNames[player.weaponType], navData.playerIndex);

                    Entity* vehicleBox = EntityManager::FindEntities("VehicleBox")[navData.playerIndex];
                    Entity* vehicle = EntityManager::FindFirstChild(vehicleBox, "Vehicle");
                    Entity* weapon = ContentManager::LoadEntity(WeaponType::turretPrefabPaths[player.weaponType], vehicle);
                    weapon->transform.SetPosition(EntityManager::FindFirstChild(vehicle, "GunTurretBase")->transform.GetLocalPosition());

                    UpdateWeaponStats(navData.playerIndex);
                } else {
                    GuiHelper::SetGuisEnabled("CharacterMenu_Arrows", false, navData.playerIndex);
					GuiHelper::SetGuisEnabled("CharacterMenu_Stats", false, navData.playerIndex);
                    GuiHelper::SetFirstGuiText("CharacterMenu_Title", "", navData.playerIndex);
                    GuiHelper::SetFirstGuiText("CharacterMenu_SubTitle", "", navData.playerIndex);
                    selected->SetText("Ready");

                    Entity* stats = EntityManager::FindEntities("CharacterMenu_Stats")[navData.playerIndex];
                    EntityManager::DestroyChildren(stats);

                    player.ready = true;
                    bool allReady = true;
                    for (int i = 0; i < Game::gameData.playerCount; ++i) {
                        allReady = Game::players[i].ready;
                        if (!allReady) break;
                    }
                    // TODO: Countdown?
                    if (allReady) StateManager::SetState(GameState_Playing);
                }
			}
		} else if (gameState == GameState_Paused) {
            GuiComponent* selected = GuiHelper::GetSelectedGui("PauseMenuButtons", navData.playerIndex);
            if (selected->ContainsText("resume")) {
                StateManager::SetState(GameState_Playing);
            } else if (selected->ContainsText("exit")) {
                Game::Instance().FinishGame();
            }
		}
	}

	if (navData.back) {
		if (gameState == GameState_Menu_Settings) {
			StateManager::SetState(GameState_Menu);
		}
		else if (gameState == GameState_Menu_Start) {
			StateManager::SetState(GameState_Menu);
		}
		else if (gameState == GameState_Menu_Start_CharacterSelect) {
            GuiComponent *selected = GuiHelper::GetSelectedGui("CharacterMenu_Buttons", navData.playerIndex);
            if (selected->ContainsText("join")) {
                StateManager::SetState(GameState_Menu_Start);
                CreateStartMenu();
            } else {
                if (GuiHelper::FirstGuiContainsText("CharacterMenu_Title", "vehicle", navData.playerIndex)) {
                    GuiHelper::SetGuisEnabled("CharacterMenu_Arrows", false, navData.playerIndex);
					GuiHelper::SetGuisEnabled("CharacterMenu_Stats", false, navData.playerIndex);
                    GuiHelper::SetFirstGuiText("CharacterMenu_Title", "", navData.playerIndex);
                    GuiHelper::SetFirstGuiText("CharacterMenu_SubTitle", "", navData.playerIndex);

                    Entity* vehicleBox = EntityManager::FindEntities("VehicleBox")[navData.playerIndex];
                    EntityManager::DestroyEntity(EntityManager::FindFirstChild(vehicleBox, "Vehicle"));

                    Entity* stats = EntityManager::FindEntities("CharacterMenu_Stats")[navData.playerIndex];
                    EntityManager::DestroyChildren(stats);

                    Game::gameData.playerCount--;
                    selected->SetText("a to join");
                } else if (GuiHelper::FirstGuiContainsText("CharacterMenu_Title", "weapon", navData.playerIndex)) {
                    GuiHelper::SetFirstGuiText("CharacterMenu_Title", "vehicle selection", navData.playerIndex);
                    GuiHelper::SetFirstGuiText("CharacterMenu_SubTitle", VehicleType::displayNames[player.vehicleType], navData.playerIndex);

                    Entity* vehicleBox = EntityManager::FindEntities("VehicleBox")[navData.playerIndex];
                    Entity* vehicle = EntityManager::FindFirstChild(vehicleBox, "Vehicle");
                    EntityManager::DestroyEntity(EntityManager::FindFirstChild(vehicle, "GunTurret"));

                    UpdateVehicleStats(navData.playerIndex);
                } else {
                    GuiHelper::SetGuisEnabled("CharacterMenu_Arrows", true, navData.playerIndex);
					GuiHelper::SetGuisEnabled("CharacterMenu_Stats", true, navData.playerIndex);
                    GuiHelper::SetFirstGuiText("CharacterMenu_Title", "weapon selection", navData.playerIndex);
                    GuiHelper::SetFirstGuiText("CharacterMenu_SubTitle", WeaponType::displayNames[player.weaponType], navData.playerIndex);
                    selected->SetText("a to continue");

                    UpdateWeaponStats(navData.playerIndex);

                    player.ready = false;
                }
            }
		}
        else if (gameState == GameState_Paused) {
            StateManager::SetState(GameState_Playing);
        }
	}

    if (navData.escape) {
        if (gameState == GameState_Playing) {
            StateManager::SetState(GameState_Paused);
            ContentManager::LoadEntity("Menu/PauseMenu.json", Game::players[navData.playerIndex].camera->GetGuiRoot());
        } else if (gameState == GameState_Paused) {
            StateManager::SetState(GameState_Playing);
        }
    }

	if (gameState == GameState_Playing) {
		if (navData.tabPressed) {
			Entity* guiRoot = Game::players[navData.playerIndex].camera->GetGuiRoot();
			Entity* leaderboard = ContentManager::LoadEntity("Menu/LeaderboardMenu.json", guiRoot);
			CreateLeaderboardMenu(leaderboard, navData.playerIndex);
		} else if (navData.tabReleased) {
			Entity* guiRoot = Game::players[navData.playerIndex].camera->GetGuiRoot();
			EntityManager::DestroyEntity(EntityManager::FindFirstChild(guiRoot, "LeaderboardMenu"));
		} else if (navData.tabHeld) {
			Entity* guiRoot = Game::players[navData.playerIndex].camera->GetGuiRoot();
			Entity* leaderboard = EntityManager::FindFirstChild(guiRoot, "LeaderboardMenu");
			if (leaderboard) {
				UpdateLeaderboardMenu(leaderboard, navData.playerIndex);
			}
		}
	}
}

void InputManager::HandleKeyboard() {
	//Keyboard Inputs
    const GameState gameState = StateManager::GetState();

	GuiNavData navData;
	navData.playerIndex = 0;

	navData.enter = Keyboard::KeyPressed(GLFW_KEY_ENTER);
	navData.back = Keyboard::KeyPressed(GLFW_KEY_ESCAPE);
		navData.escape = navData.back;
	navData.tabPressed = Keyboard::KeyPressed(GLFW_KEY_TAB);
	navData.tabHeld = Keyboard::KeyDown(GLFW_KEY_TAB);
	navData.tabReleased = Keyboard::KeyReleased(GLFW_KEY_TAB);
	
	navData.vertDir = Keyboard::KeyPressed(GLFW_KEY_UP) || Keyboard::KeyPressed(GLFW_KEY_W) ? 1
        : Keyboard::KeyPressed(GLFW_KEY_DOWN) || Keyboard::KeyPressed(GLFW_KEY_S) ? -1 : 0;
	navData.horizDir = Keyboard::KeyPressed(GLFW_KEY_RIGHT) || Keyboard::KeyPressed(GLFW_KEY_D) ? 1
        : Keyboard::KeyPressed(GLFW_KEY_LEFT) || Keyboard::KeyPressed(GLFW_KEY_A) ? -1 : 0;

    NavigateGuis(navData);

	if (gameState == GameState_Playing) {
        //Get Vehicle Component
		PlayerData& player = Game::players[0];
		VehicleComponent* vehicle = player.vehicleEntity->GetComponent<VehicleComponent>();
		WeaponComponent* weapon = player.vehicleEntity->GetComponent<WeaponComponent>();
		CameraComponent* cameraC = player.camera;

		const float forwardPower = Keyboard::KeyDown(GLFW_KEY_W) ? 1 : 0;
		const float backwardPower = Keyboard::KeyDown(GLFW_KEY_S) ? 1 : 0;

		float steer = 0;
        if (Keyboard::KeyDown(GLFW_KEY_A)) { //Steer Left
			steer += 1;
        }
        if (Keyboard::KeyDown(GLFW_KEY_D)) { //Steer Right
			steer += -1;
        }

		float handbrake = 0;
		if (Keyboard::KeyDown(GLFW_KEY_SPACE)) {
			handbrake = 1;
		}

		vehicle->HandleAcceleration( forwardPower, backwardPower);
		vehicle->Handbrake(handbrake);
		vehicle->Steer(steer);
    }
}

void InputManager::HandleVehicleControllerInput(size_t controllerNum, int &leftVibrate, int &rightVibrate) {

	XboxController *controller = xboxControllers[controllerNum];

	if (controllerNum > Game::gameData.playerCount - 1) return;
	//if (!Game::players[controllerNum].alive) return;

	bool active = controller->GetPreviousState().Gamepad.bLeftTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD ||
		controller->GetPreviousState().Gamepad.bRightTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD ||
		abs(controller->GetPreviousState().Gamepad.sThumbLX) >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ||
		abs(controller->GetPreviousState().Gamepad.sThumbLY) >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ||
		abs(controller->GetPreviousState().Gamepad.sThumbRX) >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ||
		abs(controller->GetPreviousState().Gamepad.sThumbRY) >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ||
		controller->GetState().Gamepad.wButtons ||
		controller->GetState().Gamepad.bLeftTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD ||
		controller->GetState().Gamepad.bRightTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD ||
		abs(controller->GetState().Gamepad.sThumbLX) >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ||
		abs(controller->GetState().Gamepad.sThumbLY) >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ||
		abs(controller->GetState().Gamepad.sThumbRX) >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ||
		abs(controller->GetState().Gamepad.sThumbRY) >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ||
		controller->GetPreviousState().Gamepad.wButtons;

	if (active) {
		// -------------------------------------------------------------------------------------------------------------- //
		// Get Components
		// -------------------------------------------------------------------------------------------------------------- //
		PlayerData& player = Game::players[controllerNum];
		VehicleComponent* vehicle = player.vehicleEntity->GetComponent<VehicleComponent>();
		WeaponComponent* weapon = player.vehicleEntity->GetComponent<WeaponComponent>();
		CameraComponent* cameraC = player.camera;

		// -------------------------------------------------------------------------------------------------------------- //
		// Get Button States
		// -------------------------------------------------------------------------------------------------------------- //
		int heldButtons = controller->GetState().Gamepad.wButtons & controller->GetPreviousState().Gamepad.wButtons;
		int pressedButtons = (controller->GetState().Gamepad.wButtons ^ controller->GetPreviousState().Gamepad.wButtons) & controller->GetState().Gamepad.wButtons;
		int releasedButtons = (controller->GetState().Gamepad.wButtons ^ controller->GetPreviousState().Gamepad.wButtons) & controller->GetPreviousState().Gamepad.wButtons;
		
		// -------------------------------------------------------------------------------------------------------------- //
		// Manage Speed
		// -------------------------------------------------------------------------------------------------------------- //
		float forwardPower = 0; 
		float backwardPower = 0; 
		if (abs(controller->GetState().Gamepad.bRightTrigger) >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
			forwardPower = controller->GetState().Gamepad.bRightTrigger / 255;
		}
		if (abs(controller->GetState().Gamepad.bLeftTrigger) >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
			backwardPower = controller->GetState().Gamepad.bLeftTrigger / 255;
		}

		// -------------------------------------------------------------------------------------------------------------- //
		// Manage Steering
		// -------------------------------------------------------------------------------------------------------------- //
		float steer = 0;
		if (abs(controller->GetState().Gamepad.sThumbLX) >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
			steer = -controller->GetState().Gamepad.sThumbLX / 32768.0f;
		}

		// -------------------------------------------------------------------------------------------------------------- //
		// Manage Camera
		// -------------------------------------------------------------------------------------------------------------- //
		float cameraX = 0;
		float cameraY = 0;
	
		// an attempt to reset camera behind the vehicle
		if (pressedButtons & XINPUT_GAMEPAD_RIGHT_THUMB) {
			player.follow = !player.follow;
		}
		if (player.follow) {
			glm::vec3 vehicleDirection = vehicle->GetEntity()->transform.GetForward();
			vehicleDirection.y = 0;
			vehicleDirection = glm::normalize(vehicleDirection);
			cameraX = -cameraC->GetCameraHorizontalAngle() + ((acos(glm::dot(vehicleDirection, Transform::FORWARD)))) * (glm::dot(vehicleDirection, Transform::RIGHT) > 0 ? 1 : -1) + M_PI_2;
			cameraY = -cameraC->GetCameraVerticalAngle() + M_PI * .45f;
		}

		if (abs(controller->GetState().Gamepad.sThumbRX) >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) {
			cameraX += static_cast<float>(controller->GetState().Gamepad.sThumbRX) / 32768.0f;
		}
		if (abs(controller->GetState().Gamepad.sThumbRY) >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) {
			cameraY += static_cast<float>(controller->GetState().Gamepad.sThumbRY) / 32768.0f;
		}

		if (cameraX > M_PI) cameraX -= M_PI * 2;
		if (cameraX < -M_PI) cameraX += M_PI * 2;

		// -------------------------------------------------------------------------------------------------------------- //
		// Manage Handbrake
		// -------------------------------------------------------------------------------------------------------------- //
		//LEFT-SHOULDER
		float handbrake = 0;
		if (heldButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) {
			handbrake = 1;
		}

		// -------------------------------------------------------------------------------------------------------------- //
		// Manage Shooting
		// -------------------------------------------------------------------------------------------------------------- //
		if (pressedButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) {
			weapon->Charge();
		}
		else if (heldButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) {
			PxQueryFilterData filterData;
			float rayLength = 200.0f;
			filterData.data.word0 = RaycastGroups::GetGroupsMask(vehicle->GetRaycastGroup());
			glm::vec3 cameraHit = cameraC->CastRay(rayLength, filterData);
			vehicle->GetEntity()->GetComponent<WeaponComponent>()->Shoot(cameraHit);
		}
		
		// -------------------------------------------------------------------------------------------------------------- //
		// Update
		// -------------------------------------------------------------------------------------------------------------- //
		vehicle->HandleAcceleration(forwardPower, backwardPower);
		vehicle->Handbrake(handbrake);
		vehicle->Steer(steer);

		UpdateCamera(vehicle->GetEntity(), cameraC, glm::vec2(cameraX, cameraY));
	}
}

void InputManager::HandleController() {
	//Iterate through each controller
	for (auto it = xboxControllers.begin(); it != xboxControllers.end(); ++it) {
		XboxController *controller = *it;
        if (!controller->IsConnected()) continue;
        const size_t controllerNum = controller->GetControllerNumber();

        int leftVibrate = 0;
        int rightVibrate = 0;

		//Manage Button States
		int heldButtons = controller->GetState().Gamepad.wButtons & controller->GetPreviousState().Gamepad.wButtons;
		int pressedButtons = (controller->GetState().Gamepad.wButtons ^ controller->GetPreviousState().Gamepad.wButtons) & controller->GetState().Gamepad.wButtons;
		int releasedButtons = (controller->GetState().Gamepad.wButtons ^ controller->GetPreviousState().Gamepad.wButtons) & controller->GetPreviousState().Gamepad.wButtons;
	
		GuiNavData navData;
		navData.playerIndex = controllerNum;

		navData.enter = pressedButtons & XINPUT_GAMEPAD_A;
		navData.back = pressedButtons & XINPUT_GAMEPAD_B;
		navData.escape = pressedButtons & XINPUT_GAMEPAD_START;
		navData.tabPressed = pressedButtons & XINPUT_GAMEPAD_BACK;
		navData.tabHeld = heldButtons & XINPUT_GAMEPAD_BACK;
		navData.tabReleased = releasedButtons & XINPUT_GAMEPAD_BACK;
	    
		if (abs(controller->GetPreviousState().Gamepad.sThumbLY) < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && abs(controller->GetState().Gamepad.sThumbLY) >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
			navData.vertDir = controller->GetState().Gamepad.sThumbLY / abs(controller->GetState().Gamepad.sThumbLY);
		}
		navData.vertDir = pressedButtons & XINPUT_GAMEPAD_DPAD_DOWN ? -1 : pressedButtons & XINPUT_GAMEPAD_DPAD_UP ? 1 : navData.vertDir;

		if (abs(controller->GetPreviousState().Gamepad.sThumbLX) < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && abs(controller->GetState().Gamepad.sThumbLX) >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
			navData.horizDir = controller->GetState().Gamepad.sThumbLX / abs(controller->GetState().Gamepad.sThumbLX);
		}
		navData.horizDir = pressedButtons & XINPUT_GAMEPAD_DPAD_LEFT ? -1 : pressedButtons & XINPUT_GAMEPAD_DPAD_RIGHT ? 1 : navData.horizDir;
			
		NavigateGuis(navData);

        
	    if (StateManager::GetState() == GameState_Playing) {
            HandleVehicleControllerInput(controllerNum, leftVibrate, rightVibrate);
        }

		//TODO: Get vibrate

        //Vibrate Controller
        controller->Vibrate(leftVibrate, rightVibrate);

        //Update Previous Controller State
        controller->SetPreviousState(controller->GetState());
	}
}

size_t InputManager::GetControllerCount() {
    size_t count = 0;
    for (XboxController *controller : xboxControllers) {
        if (controller->IsConnected()) {        // TODO: Fix this function
            if (++count == 4) break;
        }
    }
    return 1;
    return count;
}
