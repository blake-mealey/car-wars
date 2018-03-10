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
	float minAngle = (M_PI_4)					+ dotFU;
	float maxAngle = (M_PI_2 + (M_PI_4 / 4.0f)) + dotFU;	
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

		//Shoot Weapon
		if (Mouse::ButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
			vehicle->GetComponent<WeaponComponent>()->Charge();
		} else if (Mouse::ButtonDown(GLFW_MOUSE_BUTTON_LEFT)) {
			vehicle->GetComponent<WeaponComponent>()->Shoot();
		}

		//Cursor Inputs
		glfwSetInputMode(graphicsInstance.GetWindow(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		glm::vec2 windowSize = graphicsInstance.GetWindowSize();
		double xPos, yPos;
		Mouse::GetCursorPosition(graphicsInstance.GetWindow(), &xPos, &yPos);

		//Get Camera Component
		CameraComponent* cameraComponent = static_cast<CameraComponent*>(EntityManager::GetComponents(ComponentType_Camera)[0]);
		glm::vec2 angleDiffs = 10.f * (windowSize*0.5f - glm::vec2(xPos, yPos)) / windowSize;
		UpdateCamera(vehicle, cameraComponent, angleDiffs);

		//Set Cursor to Middle
		glfwSetCursorPos(graphicsInstance.GetWindow(), windowSize.x / 2, windowSize.y / 2);
	} else {
		//Set Cursor Visible
		glfwSetInputMode(graphicsInstance.GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void InputManager::NavigateGuis(int vertDir, int horizDir, int enter, int back, int playerIndex) {
    PlayerData& player = Game::players[playerIndex];      // TODO: From passed index

	const GameState gameState = StateManager::GetState();
	// Navigate buttons up/down
	if (vertDir != 0 && gameState >= GameState_Menu && gameState < __GameState_Menu_End) {
		std::string buttonGroupName;
		bool noNavigation = false;
		switch (gameState) {
		case GameState_Menu:
			buttonGroupName = "MainMenu_Buttons";
			break;
		case GameState_Menu_Start:
			buttonGroupName = "StartMenu_Buttons";
			break;
		case GameState_Menu_Settings:
			buttonGroupName = "OptionsMenu_Buttons";
			break;
		default:
			noNavigation = true;
		}

		if (!noNavigation) {
			if (vertDir > 0) GuiHelper::SelectPreviousGui(buttonGroupName);
			if (vertDir < 0) GuiHelper::SelectNextGui(buttonGroupName);
		}
	}

	if (horizDir != 0) {
		if (gameState == GameState_Menu_Start_CharacterSelect) {
            const int diff = horizDir < 0 ? -1 : 1;
            if (GuiHelper::FirstGuiContainsText("CharacterMenu_Title", "vehicle", playerIndex)) {
                player.vehicleType = (player.vehicleType + diff) % VehicleType::Count;
                if (player.vehicleType < 0) player.vehicleType += VehicleType::Count;
                const string text = VehicleType::displayNames[player.vehicleType];
                GuiHelper::SetFirstGuiText("CharacterMenu_SubTitle", text, playerIndex);

                Entity* vehicleBox = EntityManager::FindEntities("VehicleBox")[playerIndex];
                EntityManager::DestroyEntity(EntityManager::FindFirstChild(vehicleBox, "Vehicle"));
                ContentManager::LoadEntity(VehicleType::prefabPaths[player.vehicleType], vehicleBox);
            } else if (GuiHelper::FirstGuiContainsText("CharacterMenu_Title", "weapon", playerIndex)) {
				player.weaponType = (player.weaponType + diff) % WeaponType::Count;
				if (player.weaponType < 0) player.weaponType += WeaponType::Count;
				const string text = WeaponType::displayNames[player.weaponType];
				GuiHelper::SetFirstGuiText("CharacterMenu_SubTitle", text, playerIndex);

                Entity* vehicleBox = EntityManager::FindEntities("VehicleBox")[playerIndex];
                Entity* vehicle = EntityManager::FindFirstChild(vehicleBox, "Vehicle");
                EntityManager::DestroyEntity(EntityManager::FindFirstChild(vehicle, "GunTurret"));
                Entity* weapon = ContentManager::LoadEntity(WeaponType::turretPrefabPaths[player.weaponType], vehicle);
                weapon->transform.SetPosition(EntityManager::FindFirstChild(vehicle, "GunTurretBase")->transform.GetLocalPosition());
			}
		}
	}

	if (enter) {
		if (gameState == GameState_Menu) {
			GuiComponent *selected = GuiHelper::GetSelectedGui("MainMenu_Buttons");
			if (selected->HasText("start")) {
				StateManager::SetState(GameState_Menu_Start);
			}
			else if (selected->HasText("options")) {
				StateManager::SetState(GameState_Menu_Settings);
			}
			else if (selected->HasText("exit")) {
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
			//Press Enter to Go to Confirm
			GuiComponent *selected = GuiHelper::GetSelectedGui("StartMenu_Buttons");
			if (selected->HasText("back")) {
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
				GuiHelper::SetFirstGuiText("CharacterMenu_Title", "vehicle selection", playerIndex);
				GuiHelper::SetFirstGuiText("CharacterMenu_SubTitle", VehicleType::displayNames[player.vehicleType], playerIndex);

                Entity* vehicleBox = EntityManager::FindEntities("VehicleBox")[playerIndex];
                ContentManager::LoadEntity(VehicleType::prefabPaths[player.vehicleType], vehicleBox);

				selected->SetText("a to continue");
			} else {
                if (GuiHelper::FirstGuiContainsText("CharacterMenu_Title", "vehicle", playerIndex)) {
                    GuiHelper::SetFirstGuiText("CharacterMenu_Title", "weapon selection", playerIndex);
                    GuiHelper::SetFirstGuiText("CharacterMenu_SubTitle", WeaponType::displayNames[player.weaponType], playerIndex);

                    Entity* vehicleBox = EntityManager::FindEntities("VehicleBox")[playerIndex];
                    Entity* vehicle = EntityManager::FindFirstChild(vehicleBox, "Vehicle");
                    Entity* weapon = ContentManager::LoadEntity(WeaponType::turretPrefabPaths[player.weaponType], vehicle);
                    weapon->transform.SetPosition(EntityManager::FindFirstChild(vehicle, "GunTurretBase")->transform.GetLocalPosition());
                } else {
                    GuiHelper::SetGuisEnabled("CharacterMenu_Arrows", false, playerIndex);
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
            } else {
                if (GuiHelper::FirstGuiContainsText("CharacterMenu_Title", "vehicle", playerIndex)) {
                    GuiHelper::SetGuisEnabled("CharacterMenu_Arrows", false, playerIndex);
                    GuiHelper::SetFirstGuiText("CharacterMenu_Title", "", playerIndex);
                    GuiHelper::SetFirstGuiText("CharacterMenu_SubTitle", "", playerIndex);

                    Entity* vehicleBox = EntityManager::FindEntities("VehicleBox")[playerIndex];
                    EntityManager::DestroyEntity(EntityManager::FindFirstChild(vehicleBox, "Vehicle"));

                    Game::gameData.playerCount--;
                    selected->SetText("a to join");
                } else if (GuiHelper::FirstGuiContainsText("CharacterMenu_Title", "weapon", playerIndex)) {
                    GuiHelper::SetFirstGuiText("CharacterMenu_Title", "vehicle selection", playerIndex);
                    GuiHelper::SetFirstGuiText("CharacterMenu_SubTitle", VehicleType::displayNames[player.vehicleType], playerIndex);

                    Entity* vehicleBox = EntityManager::FindEntities("VehicleBox")[playerIndex];
                    Entity* vehicle = EntityManager::FindFirstChild(vehicleBox, "Vehicle");
                    EntityManager::DestroyEntity(EntityManager::FindFirstChild(vehicle, "GunTurret"));
                } else {
                    GuiHelper::SetGuisEnabled("CharacterMenu_Arrows", true, playerIndex);
                    GuiHelper::SetFirstGuiText("CharacterMenu_Title", "weapon selection", playerIndex);
                    GuiHelper::SetFirstGuiText("CharacterMenu_SubTitle", WeaponType::displayNames[player.weaponType], playerIndex);
                    selected->SetText("a to continue");

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
		int vertDir = Keyboard::KeyPressed(GLFW_KEY_UP) || Keyboard::KeyPressed(GLFW_KEY_W) ? 1 : Keyboard::KeyPressed(GLFW_KEY_DOWN) || Keyboard::KeyPressed(GLFW_KEY_S) ? -1 : 0;
		int horizDir = Keyboard::KeyPressed(GLFW_KEY_RIGHT) || Keyboard::KeyPressed(GLFW_KEY_D) ? 1 : Keyboard::KeyPressed(GLFW_KEY_LEFT) || Keyboard::KeyPressed(GLFW_KEY_A) ? -1 : 0;
		NavigateGuis(vertDir, horizDir, Keyboard::KeyPressed(GLFW_KEY_ENTER), Keyboard::KeyPressed(GLFW_KEY_ESCAPE), 0);
	} else if (gameState == GameState_Playing) {
        //Get Vehicle Component
        VehicleComponent* vehicle = static_cast<VehicleComponent*>(EntityManager::GetComponents(ComponentType_Vehicle)[0]);

        //Drive Forward
        if (Keyboard::KeyDown(GLFW_KEY_W)) {
            //cout << (int)(vehicle->pxVehicle->mDriveDynData.getCurrentGear() - PxVehicleGearsData::eNEUTRAL) << endl;
            if (vehicle->pxVehicle->mDriveDynData.getCurrentGear() == PxVehicleGearsData::eREVERSE) {
                vehicle->pxVehicle->mDriveDynData.forceGearChange(PxVehicleGearsData::eFIRST);
            }
            vehicle->pxVehicleInputData.setAnalogAccel(1.f);
        }
        if (Keyboard::KeyReleased(GLFW_KEY_W)) {
            vehicle->pxVehicleInputData.setAnalogAccel(0.0f);
        }
        //Reverse
        if (Keyboard::KeyDown(GLFW_KEY_S)) {
            if (vehicle->pxVehicle->mDriveDynData.getCurrentGear() == PxVehicleGearsData::eFIRST) {
                vehicle->pxVehicle->mDriveDynData.forceGearChange(PxVehicleGearsData::eREVERSE);
            }
            vehicle->pxVehicleInputData.setAnalogAccel(1.f);
        }
        if (Keyboard::KeyReleased(GLFW_KEY_S)) {
            vehicle->pxVehicleInputData.setAnalogAccel(0.0f);
        }
        //Steer Left
        if (Keyboard::KeyDown(GLFW_KEY_A)) {
            vehicle->pxVehicleInputData.setAnalogSteer(1.f);
        }
        if (Keyboard::KeyReleased(GLFW_KEY_A)) {
            vehicle->pxVehicleInputData.setAnalogSteer(0);
        }
        //Steer Right
        if (Keyboard::KeyDown(GLFW_KEY_D)) {
            vehicle->pxVehicleInputData.setAnalogSteer(-1.f);
        }
        if (Keyboard::KeyReleased(GLFW_KEY_D)) {
            vehicle->pxVehicleInputData.setAnalogSteer(0);
        }
        //Go to Pause Screen
        if (Keyboard::KeyPressed(GLFW_KEY_ESCAPE)) {
            StateManager::SetState(GameState_Paused);
        }
    } else if (gameState == GameState_Paused) {
		//Go to Game Playing
		if (Keyboard::KeyPressed(GLFW_KEY_ESCAPE)) {
			StateManager::SetState(GameState_Playing);
		}
	}
}

void InputManager::HandleVehicleControllerInput(size_t controllerNum, VehicleComponent *vehicle, int &leftVibrate, int &rightVibrate) {

	XboxController *controller = xboxControllers[controllerNum];

	if (controllerNum > Game::gameData.playerCount - 1) return;
	//if (!Game::players[controllerNum].alive) return;

	// -------------------------------------------------------------------------------------------------------------- //
	// TRIGGERS
	// -------------------------------------------------------------------------------------------------------------- //

	bool active = controller->GetState().Gamepad.bLeftTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD ||
		controller->GetState().Gamepad.bRightTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD ||
		abs(controller->GetState().Gamepad.sThumbLX) >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ||
		abs(controller->GetState().Gamepad.sThumbLY) >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ||
		abs(controller->GetState().Gamepad.sThumbRX) >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ||
		abs(controller->GetState().Gamepad.sThumbRY) >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ||
		controller->GetState().Gamepad.wButtons;

	if (active) {

		//Manage Trigger States
		//Left Trigger
		const float amountPressed = abs(controller->GetState().Gamepad.bRightTrigger - controller->GetState().Gamepad.bLeftTrigger);
		bool brake = false;
		if (controller->GetState().Gamepad.bLeftTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
			cout << "Controller: " << controller->GetControllerNumber() << " L-TRIGGER pressed" << endl;
			if (amountPressed < XINPUT_GAMEPAD_TRIGGER_THRESHOLD * 2) {
				brake = true;
				vehicle->pxVehicleInputData.setAnalogBrake(1.f);
			}
			else {
				if (vehicle->pxVehicle->computeForwardSpeed() > 5.f) {
					brake = true;
					vehicle->pxVehicleInputData.setAnalogBrake(controller->GetState().Gamepad.bLeftTrigger / 255.0f);
				}

				else {
					vehicle->pxVehicleInputData.setAnalogBrake(0.0f);
					vehicle->pxVehicle->mDriveDynData.forceGearChange(PxVehicleGearsData::eREVERSE);
				}
			}
		}
		else {
			vehicle->pxVehicleInputData.setAnalogBrake(0.0f);
		}

		//Right Trigger
		if (controller->GetState().Gamepad.bRightTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
			cout << "Controller: " << controller->GetControllerNumber() << " R-TRIGGER pressed" << endl;
			if (vehicle->pxVehicle->mDriveDynData.getCurrentGear() < PxVehicleGearsData::eFIRST) {
				vehicle->pxVehicle->mDriveDynData.forceGearChange(PxVehicleGearsData::eFIRST);
			}

		}

		if (!brake && amountPressed >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD * 2) vehicle->pxVehicleInputData.setAnalogAccel(amountPressed / 255.0f);
		else {
			vehicle->pxVehicleInputData.setAnalogAccel(0);
			vehicle->pxVehicle->mDriveDynData.forceGearChange(PxVehicleGearsData::eNEUTRAL);
		}

		// -------------------------------------------------------------------------------------------------------------- //
		// STICKS
		// -------------------------------------------------------------------------------------------------------------- //

		//Left Joystick X-Axis
		if (controller->GetState().Gamepad.sThumbLX >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE || controller->GetState().Gamepad.sThumbLX <= -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
			cout << "Controller: " << controller->GetControllerNumber() << " LEFT-JOYSTICK X-AXIS movement" << endl;

			vehicle->pxVehicleInputData.setAnalogSteer(-controller->GetState().Gamepad.sThumbLX / 32768.0f);
		}
		else {
			vehicle->pxVehicleInputData.setAnalogSteer(0.0f);
		}

		//Left Joystick Y-Axis
		if (controller->GetState().Gamepad.sThumbLY >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE || controller->GetState().Gamepad.sThumbLY <= -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
			cout << "Controller: " << controller->GetControllerNumber() << " LEFT-JOYSTICK Y-AXIS movement" << endl;
		}

		// ---- RIGHT STICK ---- //
		Entity *camera = EntityManager::FindEntities("Camera")[controllerNum];
		CameraComponent* cameraC = camera->GetComponent<CameraComponent>();

		//Right Joystick X-Axis
		if (controller->GetState().Gamepad.sThumbRX >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || controller->GetState().Gamepad.sThumbRX <= -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) {
			cout << "Controller: " << controller->GetControllerNumber() << " RIGHT-JOYSTICK X-AXIS movement" << endl;

			float x = -static_cast<float>(controller->GetState().Gamepad.sThumbRX) / 30000.f;
			UpdateCamera(vehicle->GetEntity(), cameraC, glm::vec2(x, 0.f));
		}

		//Right Joystick Y-Axis
		if (((controller->GetPreviousState().Gamepad.sThumbRY >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) || (controller->GetPreviousState().Gamepad.sThumbRY <= -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)) && ((controller->GetState().Gamepad.sThumbRY >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) || (controller->GetState().Gamepad.sThumbRY <= -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE))) {
			cout << "Controller: " << controller->GetControllerNumber() << " RIGHT-JOYSTICK Y-AXIS movement" << endl;

			float y = static_cast<float>(controller->GetState().Gamepad.sThumbRY) / 30000.f;
			UpdateCamera(vehicle->GetEntity(), cameraC, glm::vec2(0.f, y));
		}


		// -------------------------------------------------------------------------------------------------------------- //
		// BUTTONS
		// -------------------------------------------------------------------------------------------------------------- //

		//Manage Button States
		int heldButtons = controller->GetState().Gamepad.wButtons & controller->GetPreviousState().Gamepad.wButtons;
		int pressedButtons = (controller->GetState().Gamepad.wButtons ^ controller->GetPreviousState().Gamepad.wButtons) & controller->GetState().Gamepad.wButtons;
		int releasedButtons = (controller->GetState().Gamepad.wButtons ^ controller->GetPreviousState().Gamepad.wButtons) & controller->GetPreviousState().Gamepad.wButtons;

		//LEFT-SHOULDER
		if (heldButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) {
			cout << "Controller: " << controller->GetControllerNumber() << " LEFT-SHOULDER held" << endl;

			vehicle->pxVehicleInputData.setAnalogHandbrake(1.f);
		}
		else {
			vehicle->pxVehicleInputData.setAnalogHandbrake(0.f);
		}
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
            vector<Component*> vehicleComponents = EntityManager::GetComponents(ComponentType_Vehicle);
            VehicleComponent* vehicle = static_cast<VehicleComponent*>(vehicleComponents[controllerNum]);
			WeaponComponent *weapon = vehicle->GetEntity()->GetComponent<WeaponComponent>();
            //        cout << "Current speed: " << vehicle->pxVehicle->computeForwardSpeed() << endl;
            HandleVehicleControllerInput(controllerNum, vehicle, leftVibrate, rightVibrate);

			//LEFT-SHOULDER
			if (pressedButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) {
				cout << "Controller: " << controller->GetControllerNumber() << " RIGHT-SHOULDER pressed" << endl;
				weapon->Charge();
			} else if (heldButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) {
				cout << "Controller: " << controller->GetControllerNumber() << " RIGHT-SHOULDER held" << endl;
				weapon->Shoot();
			}
		}
		else if (StateManager::GetState() < __GameState_Menu_End) {
			int vertDir = 0;
			if ((controller->GetPreviousState().Gamepad.sThumbLY < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && controller->GetState().Gamepad.sThumbLY >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
				|| (controller->GetPreviousState().Gamepad.sThumbLY > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && controller->GetState().Gamepad.sThumbLY <= -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)) {
				vertDir = controller->GetState().Gamepad.sThumbLY;
			}

			int horizDir = 0;
			if ((controller->GetPreviousState().Gamepad.sThumbLX < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && controller->GetState().Gamepad.sThumbLX >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
				|| (controller->GetPreviousState().Gamepad.sThumbLX > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && controller->GetState().Gamepad.sThumbLX <= -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)) {
				horizDir = controller->GetState().Gamepad.sThumbLX;
			}

			NavigateGuis(vertDir, horizDir, pressedButtons & XINPUT_GAMEPAD_A, pressedButtons & XINPUT_GAMEPAD_B, controllerNum);
		}

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
