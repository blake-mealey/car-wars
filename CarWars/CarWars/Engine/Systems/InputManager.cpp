#include "InputManager.h"

#include "Graphics.h"
#include "Game.h"
#include "../Systems/StateManager.h"
#include "../Entities/EntityManager.h"
#include "../Components/CameraComponent.h"
#include "../Components/RigidbodyComponents/VehicleComponent.h"

#include "Physics.h"

#include "PxPhysicsAPI.h"

#include "vehicle/PxVehicleUtil.h"
#include "../Components/WeaponComponents/MachineGunComponent.h"
#include "../Components/WeaponComponents/RailGunComponent.h"
#include "../Components/GuiComponents/GuiComponent.h"
#include "GuiHelper.h"
#include "Content/ContentManager.h"

#include "../Systems/Physics/CollisionGroups.h"
#include "../Systems/Physics/RaycastGroups.h"
#include "../Systems/Physics.h"


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
	glm::vec3 vehicleForward = vehicle->transform.GetForward();
	glm::vec3 vehicleUp = vehicle->transform.GetUp();
	glm::vec3 vehicleRight = vehicle->transform.GetRight();

	float dotFR = glm::dot(vehicleForward, Transform::RIGHT);
	float dotRU = glm::dot(vehicleRight, Transform::UP);
	float dotUF = glm::dot(vehicleUp, Transform::FORWARD);

	//Update Camera Angles
	float cameraHor = camera->GetCameraHorizontalAngle();
	float cameraVer = camera->GetCameraVerticalAngle();
	float cameraSpd = camera->GetCameraSpeed();
	float cameraNewHor = (cameraHor - (angleDiffs.x * cameraSpd * StateManager::deltaTime.GetTimeSeconds()));
	float cameraNewVer = (cameraVer + (angleDiffs.y * cameraSpd * StateManager::deltaTime.GetTimeSeconds()));

	/*
	//Clamping
	float carAngleOffset = acos(glm::dot(vehicle->transform.GetUp(), Transform::UP));
	float minAngle = (M_PI_4)					+ acos(dotFU);
	float maxAngle = (M_PI_2 + (M_PI_4 / 4.0f)) + acos(dotFU);	
	cameraNewVer = glm::clamp(cameraNewVer, minAngle, maxAngle);
	*/

	camera->UpdateCameraPosition(vehicle, cameraNewHor, cameraNewVer);
	camera->SetUpVector(vehicle->transform.GetUp());

	//Get Weapon Child - Weapon Rotation
	Entity* vehicleGunTurret = EntityManager::FindFirstChild(vehicle, "GunTurret");
	float gunHor = -cameraNewHor + M_PI;
	vehicleGunTurret->transform.SetRotationAxisAngles(vehicle->transform.GetUp(), gunHor);
	float gunVer = -cameraNewVer + (M_PI_2 - (M_PI_4 / 4.0f));
	vehicleGunTurret->transform.Rotate(vehicleRight, gunVer);
}

void InputManager::HandleMouse() {
	//Mouse Inputs
	//Get Graphics Instance
	Graphics& graphicsInstance = Graphics::Instance();
	if (StateManager::GetState() == GameState_Playing) {
		//Get Vehicle Entity
		Entity* vehicle = EntityManager::FindEntities("Vehicle")[0];
		CameraComponent* cameraComponent = static_cast<CameraComponent*>(EntityManager::GetComponents(ComponentType_Camera)[0]);

		//Shoot Weapon
		if (Mouse::ButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
			vehicle->GetComponent<WeaponComponent>()->Charge();
		} else if (Mouse::ButtonDown(GLFW_MOUSE_BUTTON_LEFT)) {
			PxQueryFilterData filterData;
			filterData.data.word0 = RaycastGroups::GetGroupsMask(vehicle->GetComponent<VehicleComponent>()->GetRaycastGroup());
			glm::vec3 cameraHit = cameraComponent->CastRay(glm::length(cameraComponent->GetTarget() - cameraComponent->GetPosition()), 100, filterData);
			vehicle->GetComponent<WeaponComponent>()->Shoot(cameraHit);
		}

		//Cursor Inputs
		glfwSetInputMode(graphicsInstance.GetWindow(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		glm::vec2 windowSize = graphicsInstance.GetWindowSize();
		double xPos, yPos;
		Mouse::GetCursorPosition(graphicsInstance.GetWindow(), &xPos, &yPos);

		//Get Camera Component

		glm::vec2 angleDiffs = 10.f * (windowSize*0.5f - glm::vec2(xPos, yPos)) / windowSize;
		UpdateCamera(vehicle, cameraComponent, angleDiffs);

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
}

void NextEnumOption(Entity* option, int dir, size_t &value, size_t count, const std::string* displayNames) {
    if (!dir) return;
    dir = dir == 0 ? 0 : dir < 0 ? -1 : 1;
    if (value == 0 && dir < 0) value = count - 1;
    else if (value == count - 1 && dir > 0) value = 0;
    else value += dir;
    GuiHelper::SetSecondGuiText(option, displayNames[value]);
}

void InputManager::NavigateGuis(int vertDir, int horizDir, int enter, int back, int playerIndex) {
    // If there was no navigation, do nothing
    if (!vertDir && !horizDir && !enter && !back) return;

	bool horizontal = abs(vertDir) < abs(horizDir);
	bool vertical = abs(vertDir) > abs(horizDir);

    // Normalize inputs
    vertDir =  vertical ? vertDir / abs(vertDir) : 0;
    horizDir = horizontal ? horizDir / abs(horizDir) : 0;

    // Get the player for the current controller
    PlayerData& player = Game::players[playerIndex];      // TODO: From passed index

	const GameState gameState = StateManager::GetState();
	// Navigate buttons up/down
	if (vertDir && gameState >= GameState_Menu && gameState < __GameState_Menu_End) {
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
		default:
			noNavigation = true;
		}

		if (!noNavigation) {
			Entity* buttonGroup = EntityManager::FindEntities(buttonGroupName)[0];
            if (!entityNavigation) {
                GuiHelper::SelectNextGui(buttonGroup, -vertDir);
            } else {
                GuiHelper::SelectNextEntity(buttonGroup, -vertDir);
            }
		}
	}

	if (horizDir) {
        if (gameState == GameState_Menu_Start) {
            Entity* selected = GuiHelper::GetSelectedEntity("StartMenu_Options");
            if (GuiHelper::FirstGuiContainsText(selected, "lives")) {
                NextNumberOption(selected, horizDir, Game::gameData.numberOfLives, GameData::MIN_NUMBER_OF_LIVES, GameData::MAX_NUMBER_OF_LIVES);
            } else if (GuiHelper::FirstGuiContainsText(selected, "ai")) {
                NextNumberOption(selected, horizDir, Game::gameData.aiCount, GameData::MIN_AI_COUNT, GameData::MAX_AI_COUNT);
            } else if (GuiHelper::FirstGuiContainsText(selected, "map")) {
                NextEnumOption(selected, horizDir, Game::gameData.map, MapType::Count, MapType::displayNames);
            } else if (GuiHelper::FirstGuiContainsText(selected, "time")) {
                NextNumberOption(selected, horizDir, Game::gameData.timeLimitMinutes, GameData::MIN_TIME_LIMIT_MINUTES, GameData::MAX_TIME_LIMIT_MINUTES, " minutes");
            } else if (GuiHelper::FirstGuiContainsText(selected, "kill")) {
                NextNumberOption(selected, horizDir, Game::gameData.killLimit, GameData::MIN_KILL_LIMIT, GameData::MAX_KILL_LIMIT);
            } else if (GuiHelper::FirstGuiContainsText(selected, "game")) {
                NextEnumOption(selected, horizDir, Game::gameData.gameMode, GameModeType::Count, GameModeType::displayNames);
            }
        } else if (gameState == GameState_Menu_Start_CharacterSelect) {
            if (GuiHelper::FirstGuiContainsText("CharacterMenu_Title", "vehicle", playerIndex)) {
                player.vehicleType = (player.vehicleType + horizDir) % VehicleType::Count;
                if (player.vehicleType < 0) player.vehicleType += VehicleType::Count;
                const string text = VehicleType::displayNames[player.vehicleType];
                GuiHelper::SetFirstGuiText("CharacterMenu_SubTitle", text, playerIndex);

                Entity* vehicleBox = EntityManager::FindEntities("VehicleBox")[playerIndex];
                EntityManager::DestroyEntity(EntityManager::FindFirstChild(vehicleBox, "Vehicle"));
                ContentManager::LoadEntity(VehicleType::prefabPaths[player.vehicleType], vehicleBox);

                UpdateVehicleStats(playerIndex);
            } else if (GuiHelper::FirstGuiContainsText("CharacterMenu_Title", "weapon", playerIndex)) {
				player.weaponType = (player.weaponType + horizDir) % WeaponType::Count;
				if (player.weaponType < 0) player.weaponType += WeaponType::Count;
				const string text = WeaponType::displayNames[player.weaponType];
				GuiHelper::SetFirstGuiText("CharacterMenu_SubTitle", text, playerIndex);

                Entity* vehicleBox = EntityManager::FindEntities("VehicleBox")[playerIndex];
                Entity* vehicle = EntityManager::FindFirstChild(vehicleBox, "Vehicle");
                EntityManager::DestroyEntity(EntityManager::FindFirstChild(vehicle, "GunTurret"));
                Entity* weapon = ContentManager::LoadEntity(WeaponType::turretPrefabPaths[player.weaponType], vehicle);
                weapon->transform.SetPosition(EntityManager::FindFirstChild(vehicle, "GunTurretBase")->transform.GetLocalPosition());
				
                UpdateWeaponStats(playerIndex);
			}
		}
	}

	if (enter) {
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
			GuiComponent *selected = GuiHelper::GetSelectedGui("CharacterMenu_Buttons", playerIndex);
			if (selected->ContainsText("join")) {
                Game::gameData.playerCount++;

				GuiHelper::SetGuisEnabled("CharacterMenu_Arrows", true, playerIndex);
				GuiHelper::SetGuisEnabled("CharacterMenu_Stats", true, playerIndex);
				GuiHelper::SetFirstGuiText("CharacterMenu_Title", "vehicle selection", playerIndex);
				GuiHelper::SetFirstGuiText("CharacterMenu_SubTitle", VehicleType::displayNames[player.vehicleType], playerIndex);

                Entity* vehicleBox = EntityManager::FindEntities("VehicleBox")[playerIndex];
                ContentManager::LoadEntity(VehicleType::prefabPaths[player.vehicleType], vehicleBox);

                UpdateVehicleStats(playerIndex);

				selected->SetText("a to continue");
			} else {
                if (GuiHelper::FirstGuiContainsText("CharacterMenu_Title", "vehicle", playerIndex)) {
                    GuiHelper::SetFirstGuiText("CharacterMenu_Title", "weapon selection", playerIndex);
                    GuiHelper::SetFirstGuiText("CharacterMenu_SubTitle", WeaponType::displayNames[player.weaponType], playerIndex);

                    Entity* vehicleBox = EntityManager::FindEntities("VehicleBox")[playerIndex];
                    Entity* vehicle = EntityManager::FindFirstChild(vehicleBox, "Vehicle");
                    Entity* weapon = ContentManager::LoadEntity(WeaponType::turretPrefabPaths[player.weaponType], vehicle);
                    weapon->transform.SetPosition(EntityManager::FindFirstChild(vehicle, "GunTurretBase")->transform.GetLocalPosition());

                    UpdateWeaponStats(playerIndex);
                } else {
                    GuiHelper::SetGuisEnabled("CharacterMenu_Arrows", false, playerIndex);
					GuiHelper::SetGuisEnabled("CharacterMenu_Stats", false, playerIndex);
                    GuiHelper::SetFirstGuiText("CharacterMenu_Title", "", playerIndex);
                    GuiHelper::SetFirstGuiText("CharacterMenu_SubTitle", "", playerIndex);
                    selected->SetText("Ready");

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
		}
	}

	if (back) {
		if (gameState == GameState_Menu_Settings) {
			StateManager::SetState(GameState_Menu);
		}
		else if (gameState == GameState_Menu_Start) {
			StateManager::SetState(GameState_Menu);
		}
		else if (gameState == GameState_Menu_Start_CharacterSelect) {
            GuiComponent *selected = GuiHelper::GetSelectedGui("CharacterMenu_Buttons", playerIndex);
            if (selected->ContainsText("join")) {
                StateManager::SetState(GameState_Menu_Start);
                CreateStartMenu();
            } else {
                if (GuiHelper::FirstGuiContainsText("CharacterMenu_Title", "vehicle", playerIndex)) {
                    GuiHelper::SetGuisEnabled("CharacterMenu_Arrows", false, playerIndex);
					GuiHelper::SetGuisEnabled("CharacterMenu_Stats", false, playerIndex);
                    GuiHelper::SetFirstGuiText("CharacterMenu_Title", "", playerIndex);
                    GuiHelper::SetFirstGuiText("CharacterMenu_SubTitle", "", playerIndex);

                    Entity* vehicleBox = EntityManager::FindEntities("VehicleBox")[playerIndex];
                    EntityManager::DestroyEntity(EntityManager::FindFirstChild(vehicleBox, "Vehicle"));

                    Entity* stats = EntityManager::FindEntities("CharacterMenu_Stats")[playerIndex];
                    EntityManager::DestroyChildren(stats);

                    Game::gameData.playerCount--;
                    selected->SetText("a to join");
                } else if (GuiHelper::FirstGuiContainsText("CharacterMenu_Title", "weapon", playerIndex)) {
                    GuiHelper::SetFirstGuiText("CharacterMenu_Title", "vehicle selection", playerIndex);
                    GuiHelper::SetFirstGuiText("CharacterMenu_SubTitle", VehicleType::displayNames[player.vehicleType], playerIndex);

                    Entity* vehicleBox = EntityManager::FindEntities("VehicleBox")[playerIndex];
                    Entity* vehicle = EntityManager::FindFirstChild(vehicleBox, "Vehicle");
                    EntityManager::DestroyEntity(EntityManager::FindFirstChild(vehicle, "GunTurret"));

                    UpdateVehicleStats(playerIndex);
                } else {
                    GuiHelper::SetGuisEnabled("CharacterMenu_Arrows", true, playerIndex);
					GuiHelper::SetGuisEnabled("CharacterMenu_Stats", true, playerIndex);
                    GuiHelper::SetFirstGuiText("CharacterMenu_Title", "weapon selection", playerIndex);
                    GuiHelper::SetFirstGuiText("CharacterMenu_SubTitle", WeaponType::displayNames[player.weaponType], playerIndex);
                    selected->SetText("a to continue");

                    UpdateWeaponStats(playerIndex);

                    player.ready = false;
                }
            }
		}
	}
}

void InputManager::HandleKeyboard() {
	//Keyboard Inputs

    const GameState gameState = StateManager::GetState();

	if (gameState < __GameState_Menu_End) {
		int vertDir = Keyboard::KeyPressed(GLFW_KEY_UP) || Keyboard::KeyPressed(GLFW_KEY_W) ? 1 
			: Keyboard::KeyPressed(GLFW_KEY_DOWN) || Keyboard::KeyPressed(GLFW_KEY_S) ? -1 : 0;

		int horizDir = Keyboard::KeyPressed(GLFW_KEY_RIGHT) || Keyboard::KeyPressed(GLFW_KEY_D) ? 1 
			: Keyboard::KeyPressed(GLFW_KEY_LEFT) || Keyboard::KeyPressed(GLFW_KEY_A) ? -1 : 0;

		NavigateGuis(vertDir, horizDir, Keyboard::KeyPressed(GLFW_KEY_ENTER), Keyboard::KeyPressed(GLFW_KEY_ESCAPE), 0);

	} else if (gameState == GameState_Playing) {
        //Get Vehicle Component
        VehicleComponent* vehicle = static_cast<VehicleComponent*>(EntityManager::GetComponents(ComponentType_Vehicle)[0]);

		float forwardPower = Keyboard::KeyPressed(GLFW_KEY_W)? 1 : 0;
		float backwardPower = Keyboard::KeyPressed(GLFW_KEY_S) ? 1 : 0;

		float steer = 0;
        if (Keyboard::KeyPressed(GLFW_KEY_A)) { //Steer Left
			steer += 1;
        }
        if (Keyboard::KeyPressed(GLFW_KEY_D)) { //Steer Right
			steer += -1;
        }

		float handbrake = 0;
		if (Keyboard::KeyPressed(GLFW_KEY_SPACE)) {
			handbrake = 1;
		}

        //Go to Pause Screen
        if (Keyboard::KeyPressed(GLFW_KEY_ESCAPE)) {
			//TODO: pause();
            StateManager::SetState(GameState_Paused);
        }

		vehicle->HandleAcceleration( forwardPower, backwardPower);
		vehicle->Handbrake(handbrake);
		vehicle->Steer(steer);

    } else if (gameState == GameState_Paused) {
		//Go to Game Playing
		if (Keyboard::KeyPressed(GLFW_KEY_ESCAPE)) {
			StateManager::SetState(GameState_Playing);
		}
	}
}

void InputManager::HandleVehicleControllerInput(size_t controllerNum, int &leftVibrate, int &rightVibrate) {

	XboxController *controller = xboxControllers[controllerNum];

	if (controllerNum > Game::gameData.playerCount - 1) return;
	//if (!Game::players[controllerNum].alive) return;

	// -------------------------------------------------------------------------------------------------------------- //
	// TRIGGERS
	// -------------------------------------------------------------------------------------------------------------- //

	bool active = controller->GetPreviousState().Gamepad.bLeftTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD ||
		controller->GetPreviousState().Gamepad.bRightTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD ||
		abs(controller->GetPreviousState().Gamepad.sThumbLX) >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ||
		abs(controller->GetPreviousState().Gamepad.sThumbLY) >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ||
		abs(controller->GetPreviousState().Gamepad.sThumbRX) >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ||
		abs(controller->GetPreviousState().Gamepad.sThumbRY) >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ||
		controller->GetPreviousState().Gamepad.wButtons;

	if (active) {
		VehicleComponent* vehicle = static_cast<VehicleComponent*>(EntityManager::GetComponents(ComponentType_Vehicle)[controllerNum]); // maybe a better way to access entity
		WeaponComponent* weapon = vehicle->GetEntity()->GetComponent<WeaponComponent>();
		Entity *camera = EntityManager::FindEntities("Camera")[controllerNum];
		CameraComponent* cameraC = camera->GetComponent<CameraComponent>();
		
		// -------------------------------------------------------------------------------------------------------------- //
		// Manage Triggers
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
		// Manage Left Stick
		// -------------------------------------------------------------------------------------------------------------- //
		float steer = 0;
		if (abs(controller->GetState().Gamepad.sThumbLX) >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
			steer = -controller->GetState().Gamepad.sThumbLX / 32768.0f;
		}


		// -------------------------------------------------------------------------------------------------------------- //
		// Manage Right Stick
		// -------------------------------------------------------------------------------------------------------------- //
		float x = 0;
		float y = 0;

		if (abs(controller->GetState().Gamepad.sThumbRX) >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) {
			x = -static_cast<float>(controller->GetState().Gamepad.sThumbRX) / 30000.f;
		}
		if (abs(controller->GetState().Gamepad.sThumbRY) >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) {
			y = static_cast<float>(controller->GetState().Gamepad.sThumbRY) / 30000.f;
		}

		// -------------------------------------------------------------------------------------------------------------- //
		// BUTTONS
		// -------------------------------------------------------------------------------------------------------------- //

		//Manage Button States
		int heldButtons = controller->GetState().Gamepad.wButtons & controller->GetPreviousState().Gamepad.wButtons;
		int pressedButtons = (controller->GetState().Gamepad.wButtons ^ controller->GetPreviousState().Gamepad.wButtons) & controller->GetState().Gamepad.wButtons;
		int releasedButtons = (controller->GetState().Gamepad.wButtons ^ controller->GetPreviousState().Gamepad.wButtons) & controller->GetPreviousState().Gamepad.wButtons;

		//LEFT-SHOULDER
		float handbrake = 0;
		if (heldButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) {
			handbrake = 1;
		}

		//RIGHT-SHOULDER
		if (pressedButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) {
			weapon->Charge();
		}
		else if (heldButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) {
			PxQueryFilterData filterData;
			filterData.data.word0 = RaycastGroups::GetGroupsMask(vehicle->GetRaycastGroup());
			glm::vec3 cameraHit = cameraC->CastRay(glm::length(cameraC->GetTarget() - cameraC->GetPosition()),
				100, filterData);
			vehicle->GetEntity()->GetComponent<WeaponComponent>()->Shoot(cameraHit);
		}
		
		// an attempt to reset camera behind the vehicle
		if (pressedButtons & XINPUT_GAMEPAD_RIGHT_THUMB) {
			x = -cameraC->GetCameraHorizontalAngle();
			y = -cameraC->GetCameraVerticalAngle() + M_PI / 3;
		}

		vehicle->HandleAcceleration(forwardPower, backwardPower);
		vehicle->Handbrake(handbrake);
		vehicle->Steer(steer);

		UpdateCamera(vehicle->GetEntity(), cameraC, glm::vec2(x, y));
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

        if (StateManager::GetState() == GameState_Playing) {
            HandleVehicleControllerInput(controllerNum, leftVibrate, rightVibrate);
		}
		else if (StateManager::GetState() < __GameState_Menu_End) {
			int vertDir = 0;
			if (abs(controller->GetPreviousState().Gamepad.sThumbLY) < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && abs(controller->GetState().Gamepad.sThumbLY) >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
				vertDir = controller->GetState().Gamepad.sThumbLY / abs(controller->GetState().Gamepad.sThumbLY);
			}
			if (pressedButtons & XINPUT_GAMEPAD_DPAD_UP) {
				vertDir = 1;
			}
			if (pressedButtons & XINPUT_GAMEPAD_DPAD_DOWN) {
				vertDir = -1;
			}

			int horizDir = 0;
			if (abs(controller->GetPreviousState().Gamepad.sThumbLX) < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && abs(controller->GetState().Gamepad.sThumbLX) >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
				horizDir = controller->GetState().Gamepad.sThumbLX / abs(controller->GetState().Gamepad.sThumbLX);
			}
			if (pressedButtons & XINPUT_GAMEPAD_DPAD_LEFT) {
				horizDir = -1;
			}
			if (pressedButtons & XINPUT_GAMEPAD_DPAD_RIGHT) {
				horizDir = 1;
			}


			NavigateGuis(vertDir, horizDir, pressedButtons & XINPUT_GAMEPAD_A, pressedButtons & XINPUT_GAMEPAD_B, controllerNum);
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
