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

void InputManager::HandleMouse() {
	//Mouse Inputs

	//Get Vehicle Component
	VehicleComponent* vehicle = static_cast<VehicleComponent*>(EntityManager::GetComponents(ComponentType_Vehicle)[0]);
	//Get Graphics Instance
	Graphics& graphicsInstance = Graphics::Instance();
	//Get Camera Component
	CameraComponent* cameraComponent = static_cast<CameraComponent*>(EntityManager::GetComponents(ComponentType_Camera)[0]);
	switch (StateManager::GetState()) {
	case GameState_Playing:
		//Shoot Weapon
		if (Mouse::ButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
			static_cast<WeaponComponent*>(vehicle->GetEntity()->components[1])->Charge();
		}
		if (Mouse::ButtonDown(GLFW_MOUSE_BUTTON_LEFT)) {
			static_cast<WeaponComponent*>(vehicle->GetEntity()->components[1])->Shoot();
		}

		//Cursor Inputs
		glfwSetInputMode(graphicsInstance.GetWindow(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		int width, height;
		glfwGetFramebufferSize(graphicsInstance.GetWindow(), &width, &height);
		double xPos, yPos;
		Mouse::GetCursorPosition(graphicsInstance.GetWindow(), &xPos, &yPos);

		cameraComponent->SetCameraHorizontalAngle((cameraComponent->GetCameraHorizontalAngle() - ((float)(width / 2.0f) - xPos) * cameraComponent->GetCameraSpeed() * StateManager::deltaTime.GetTimeSeconds()));
		cameraComponent->SetCameraVerticalAngle(cameraComponent->GetCameraVerticalAngle() + ((float)(height / 2.0f) - yPos) * cameraComponent->GetCameraSpeed() * StateManager::deltaTime.GetTimeSeconds());
		
		float maxAngle, minAngle;
		maxAngle = M_PI / 2.0f;
		minAngle = (2.0f / 3.0f) * M_PI / 2.0f;

		if (cameraComponent->GetCameraVerticalAngle() < (minAngle)) {
			cameraComponent->SetCameraVerticalAngle(minAngle);
		} else if (cameraComponent->GetCameraVerticalAngle() > (maxAngle)) {
			cameraComponent->SetCameraVerticalAngle(maxAngle);
		}
		
		glfwSetCursorPos(graphicsInstance.GetWindow(), width / 2, height / 2);
		break;
	default:
		glfwSetInputMode(graphicsInstance.GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		break;
	}
}

void InputManager::HandleKeyboard() {
	//Keyboard Inputs

	//Get Vehicle Component
	VehicleComponent* vehicle = static_cast<VehicleComponent*>(EntityManager::GetComponents(ComponentType_Vehicle)[0]);
	//Switch on Game State
	switch (StateManager::GetState()) {
	case GameState_Menu:
		//Move Up Menu
		if (Keyboard::KeyPressed(GLFW_KEY_UP) || Keyboard::KeyPressed(GLFW_KEY_W)) {
			switch (StateManager::menuIndex) {
			case 0:
				StateManager::menuIndex = 2;
				break;
			default :
				StateManager::menuIndex--;
				break;
			}
			std::cout << "Game State: " << StateManager::GameStateToString() << " Index: " << StateManager::menuIndex << std::endl;
		} 
		//Move Down Menu
		if (Keyboard::KeyPressed(GLFW_KEY_DOWN) || Keyboard::KeyPressed(GLFW_KEY_S)) {
			switch (StateManager::menuIndex) {
			case 2:
				StateManager::menuIndex = 0;
				break;
			default:
				StateManager::menuIndex++;
				break;
			}
			std::cout << "Game State: " << StateManager::GameStateToString() << " Index: " << StateManager::menuIndex << std::endl;
		}
		//Enter Selection
		if (Keyboard::KeyPressed(GLFW_KEY_ENTER)) {
			switch (StateManager::menuIndex) {
			case 0:
				StateManager::menuIndex = 0;
				StateManager::SetState(GameState_Menu_Start);
				break;
			case 1:
				StateManager::menuIndex = 0;
				StateManager::SetState(GameState_Menu_Settings);
				break;
			case 2:
				Graphics& graphicsInstance = Graphics::Instance();
				glfwSetWindowShouldClose(graphicsInstance.GetWindow(), true);
				break;
			}
			std::cout << "Game State: " << StateManager::GameStateToString() << " Index: " << StateManager::menuIndex << std::endl;
		}
		//Close Game
		//TODO: Remove Later
		if (Keyboard::KeyPressed(GLFW_KEY_ESCAPE)) {
			Graphics& graphicsInstance = Graphics::Instance();
			glfwSetWindowShouldClose(graphicsInstance.GetWindow(), true);
		}
		break;
	case GameState_Menu_Settings:
		break;
	case GameState_Menu_Start:
		//Move Up Menu
		if (Keyboard::KeyPressed(GLFW_KEY_UP) || Keyboard::KeyPressed(GLFW_KEY_W)) {
			switch (StateManager::menuIndex) {
			case 0:
				StateManager::menuIndex = 5;
				break;
			default:
				StateManager::menuIndex--;
				break;
			}
			std::cout << "Game State: " << StateManager::GameStateToString() << " Index: " << StateManager::menuIndex << std::endl;
		}
		//Move Down Menu
		if (Keyboard::KeyPressed(GLFW_KEY_DOWN) || Keyboard::KeyPressed(GLFW_KEY_S)) {
			switch (StateManager::menuIndex) {
			case 5:
				StateManager::menuIndex = 0;
				break;
			default:
				StateManager::menuIndex++;
				break;
			}
			std::cout << "Game State: " << StateManager::GameStateToString() << " Index: " << StateManager::menuIndex << std::endl;
		}
		//Change Value Left
		if (Keyboard::KeyPressed(GLFW_KEY_LEFT) || Keyboard::KeyPressed(GLFW_KEY_A)) {
			Game& gameInstance = Game::Instance();
			size_t mapIndex;
			size_t gameModeIndex;
			switch (StateManager::menuIndex) {
			case 0:
				mapIndex = (int)gameInstance.selectedMap;
				switch (mapIndex) {
				case 0:
					mapIndex = 0;
					break;
				default:
					mapIndex--;
					break;
				}
				gameInstance.selectedMap = static_cast<Map>(mapIndex);
				std::cout << "Map Selected: " << gameInstance.MapToString() << std::endl;
				break;
			case 1:
				gameModeIndex = (int)gameInstance.selectedGameMode;
				switch (gameModeIndex) {
				case 0:
					gameModeIndex = 1;
					break;
				default:
					gameModeIndex--;
					break;
				}
				gameInstance.selectedGameMode = static_cast<GameMode>(gameModeIndex);
				std::cout << "Game Mode Selected: " << gameInstance.GameModeToString() << std::endl;
				break;
			case 2:
				switch (gameInstance.numberOfAi) {
				case 0:
					gameInstance.numberOfAi = 7;
					break;
				default:
					gameInstance.numberOfAi--;
					break;
				}
				std::cout << "Number of AI: " << gameInstance.numberOfAi << std::endl;
				break;
			case 3:
				switch (gameInstance.numberOfLives) {
				case 1:
					gameInstance.numberOfLives = INFINITE;
					break;
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
				case 10:
					gameInstance.numberOfLives--;
					break;
				case INFINITE:
					gameInstance.numberOfLives = 50;
					break;
				default:
					gameInstance.numberOfLives -= 5;
					break;
				}
				std::cout << "Number of Lives: " << gameInstance.numberOfLives << std::endl;
				break;
			case 4:
				switch (gameInstance.killLimit) {
				case 1:
					gameInstance.killLimit = INFINITE;
					break;
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
				case 10:
					gameInstance.killLimit--;
					break;
				case INFINITE:
					gameInstance.killLimit = 100;
					break;
				default:
					gameInstance.killLimit -= 5;
					break;
				}
				std::cout << "Kill Limit: " << gameInstance.killLimit << std::endl;
				break;
			case 5:
				switch (gameInstance.timeLimitMinutes) {
				case 5:
					gameInstance.timeLimitMinutes = INFINITE;
					break;
				case 6:
				case 7:
				case 8:
				case 9:
				case 10:
					gameInstance.timeLimitMinutes--;
					break;
				case INFINITE:
					gameInstance.timeLimitMinutes = 60;
					break;
				default:
					gameInstance.timeLimitMinutes -= 5;
					break;
				}
				std::cout << "Time Limit: " << gameInstance.timeLimitMinutes << std::endl;
				break;
			}
		}
		//Change Value Right
		if (Keyboard::KeyPressed(GLFW_KEY_RIGHT) || Keyboard::KeyPressed(GLFW_KEY_D)) {
			Game& gameInstance = Game::Instance();
			size_t mapIndex;
			size_t gameModeIndex;
			switch (StateManager::menuIndex) {
			case 0:
				mapIndex = (int)gameInstance.selectedMap;
				switch (mapIndex) {
				case 0:
					mapIndex = 0;
					break;
				default:
					mapIndex++;
					break;
				}
				gameInstance.selectedMap = static_cast<Map>(mapIndex);
				std::cout << "Map Selected: " << gameInstance.MapToString() << std::endl;
				break;
			case 1:
				gameModeIndex = (int)gameInstance.selectedGameMode;
				switch (gameModeIndex) {
				case 1:
					gameModeIndex = 0;
					break;
				default:
					gameModeIndex++;
					break;
				}
				gameInstance.selectedGameMode = static_cast<GameMode>(gameModeIndex);
				std::cout << "Game Mode Selected: " << gameInstance.GameModeToString() << std::endl;
				break;
			case 2:
				switch (gameInstance.numberOfAi) {
				case 7:
					gameInstance.numberOfAi = 0;
					break;
				default:
					gameInstance.numberOfAi++;
					break;
				}
				std::cout << "Number of AI: " << gameInstance.numberOfAi << std::endl;
				break;
			case 3:
				switch (gameInstance.numberOfLives) {
				case INFINITE:
					gameInstance.numberOfLives = 1;
					break;
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
					gameInstance.numberOfLives++;
					break;
				case 50:
					gameInstance.numberOfLives = INFINITE;
					break;
				default:
					gameInstance.numberOfLives += 5;
					break;
				}
				std::cout << "Number of Lives: " << gameInstance.numberOfLives << std::endl;
				break;
			case 4:
				switch (gameInstance.killLimit) {
				case INFINITE:
					gameInstance.killLimit = 1;
					break;
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
					gameInstance.killLimit++;
					break;
				case 100:
					gameInstance.killLimit = INFINITE;
					break;
				default:
					gameInstance.killLimit += 5;
					break;
				}
				std::cout << "Kill Limit: " << gameInstance.killLimit << std::endl;
				break;
			case 5:
				switch (gameInstance.timeLimitMinutes) {
				case INFINITE:
					gameInstance.timeLimitMinutes = 5;
					break;
				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
					gameInstance.timeLimitMinutes++;
					break;
				case 60:
					gameInstance.timeLimitMinutes = INFINITE;
					break;
				default:
					gameInstance.timeLimitMinutes += 5;
					break;
				}
				std::cout << "Time Limit: " << gameInstance.timeLimitMinutes << std::endl;
				break;
			}
		}
		//Press Enter to Go to Confirm
		if (Keyboard::KeyPressed(GLFW_KEY_ENTER)) {
			StateManager::menuIndex = 0;
			StateManager::SetState(GameState_Menu_Start_CharacterSelect);
			std::cout << "Game State: " << StateManager::GameStateToString() << " Index: " << StateManager::menuIndex << std::endl;
		}
		//Press Escape to Go Back a Screen
		if (Keyboard::KeyPressed(GLFW_KEY_ESCAPE)) {
			StateManager::menuIndex = 0;
			StateManager::SetState(GameState_Menu);
			std::cout << "Game State: " << StateManager::GameStateToString() << " Index: " << StateManager::menuIndex << std::endl;
		}
		break;
	case GameState_Menu_Start_CharacterSelect:
		//TODO: Basically all of This, It is Different, Because it is Player Specific
		//TODO: Current Just Goes to Game
		if (Keyboard::KeyPressed(GLFW_KEY_ENTER)) {
			StateManager::SetState(GameState_Playing);
			std::cout << "Game State: " << StateManager::GameStateToString() << " Index: " << StateManager::menuIndex << std::endl;
		}
		//Return to Previous Screen
		if (Keyboard::KeyPressed(GLFW_KEY_ESCAPE)) {
			StateManager::menuIndex = 0;
			StateManager::SetState(GameState_Menu_Start);
			std::cout << "Game State: " << StateManager::GameStateToString() << " Index: " << StateManager::menuIndex << std::endl;
		}
		break;
	case GameState_Playing:
		//Drive Forward
		if (Keyboard::KeyDown(GLFW_KEY_W)) {
			cout << (int)(vehicle->pxVehicle->mDriveDynData.getCurrentGear() - PxVehicleGearsData::eNEUTRAL) << endl;
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
			vehicle->pxVehicle->mDriveDynData.forceGearChange(PxVehicleGearsData::eREVERSE);
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
			StateManager::menuIndex = 0;
			StateManager::SetState(GameState_Paused);
			std::cout << "Game State: " << StateManager::GameStateToString() << " Index: " << StateManager::menuIndex << std::endl;
		}
		break;
	case GameState_Paused:

		//Go to Game Playing
		if (Keyboard::KeyPressed(GLFW_KEY_ESCAPE)) {
			StateManager::menuIndex = 0;
			StateManager::SetState(GameState_Playing);
			std::cout << "Game State: " << StateManager::GameStateToString() << " Index: " << StateManager::menuIndex << std::endl;
		}
		break;
	}
}

void InputManager::HandleController() {
	//Iterate through each controller
	for (auto controller = xboxControllers.begin(); controller != xboxControllers.end(); controller++) {
		//If controller is connected
		if ((*controller)->IsConnected()) {
			int leftVibrate = 0;
			int rightVibrate = 0;
			int controllerNum = (*controller)->GetControllerNumber();

			// -------------------------------------------------------------------------------------------------------------- //
			// TRIGGERS
			// -------------------------------------------------------------------------------------------------------------- //

			//Manage Trigger States
			//Left Trigger
			if ((*controller)->GetPreviousState().Gamepad.bLeftTrigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD 
				&& (*controller)->GetState().Gamepad.bLeftTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " pressed L-TRIGGER" << endl;
			} else if ((*controller)->GetPreviousState().Gamepad.bLeftTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD 
				&& (*controller)->GetState().Gamepad.bLeftTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " held L-TRIGGER" << endl;
				//leftVibrate = 30000 * (*controller)->GetState().Gamepad.bLeftTrigger / 255;

				vector<Component*> vehicleComponents = EntityManager::GetComponents(ComponentType_Vehicle);
				VehicleComponent* vehicle = static_cast<VehicleComponent*>(vehicleComponents[controllerNum]);
				
				if (vehicle->pxVehicle->computeForwardSpeed() > 5.f) {
					vehicle->pxVehicleInputData.setAnalogBrake(1.0f);
				}
				else {
					vehicle->pxVehicleInputData.setAnalogBrake(0.0f);
					vehicle->pxVehicle->mDriveDynData.forceGearChange(PxVehicleGearsData::eREVERSE);
					vehicle->pxVehicleInputData.setAnalogAccel((*controller)->GetState().Gamepad.bLeftTrigger / 255.0f);
				}
			} else if ((*controller)->GetPreviousState().Gamepad.bLeftTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD && (*controller)->GetState().Gamepad.bLeftTrigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " released L-TRIGGER" << endl;

				vector<Component*> vehicleComponents = EntityManager::GetComponents(ComponentType_Vehicle);
				VehicleComponent* vehicle = static_cast<VehicleComponent*>(vehicleComponents[controllerNum]);
				vehicle->pxVehicleInputData.setAnalogAccel(0.0f);
			}

			//Right Trigger
			if ((*controller)->GetPreviousState().Gamepad.bRightTrigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD && (*controller)->GetState().Gamepad.bRightTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " pressed R-TRIGGER" << endl;
			} else if ((*controller)->GetPreviousState().Gamepad.bRightTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD && (*controller)->GetState().Gamepad.bRightTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " held R-TRIGGER" << endl;
				//rightVibrate = 30000 * (*controller)->GetState().Gamepad.bRightTrigger / 255;

				vector<Component*> vehicleComponents = EntityManager::GetComponents(ComponentType_Vehicle);
				VehicleComponent* vehicle = static_cast<VehicleComponent*>(vehicleComponents[controllerNum]);


				cout << vehicle->pxVehicle->computeForwardSpeed() << endl;
				vehicle->pxVehicleInputData.setAnalogBrake(0.0f);
				int speed = (int)vehicle->pxVehicle->computeForwardSpeed();
				if (vehicle->pxVehicle->mDriveDynData.getCurrentGear() == PxVehicleGearsData::eREVERSE){
					vehicle->pxVehicle->mDriveDynData.forceGearChange(PxVehicleGearsData::eFIRST);
				}

				vehicle->pxVehicleInputData.setAnalogAccel(((*controller)->GetState().Gamepad.bRightTrigger - (*controller)->GetState().Gamepad.bLeftTrigger) / 255.0f);
				
				//Entity *boulder = EntityManager::FindEntities("Boulder")[0];
				//float x = 0.05f * (*controller)->GetState().Gamepad.bRightTrigger;
				//boulder->transform.Translate(boulder->transform.GetForward() * dt.GetTimeSeconds() * x);

			} else if ((*controller)->GetPreviousState().Gamepad.bRightTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD && (*controller)->GetState().Gamepad.bRightTrigger < XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " released R-TRIGGER" << endl;

				vector<Component*> vehicleComponents = EntityManager::GetComponents(ComponentType_Vehicle);
				VehicleComponent* vehicle = static_cast<VehicleComponent*>(vehicleComponents[controllerNum]);
				vehicle->pxVehicleInputData.setAnalogAccel(0.0f);
			}

			//Left Joystick X-Axis
			if (((*controller)->GetPreviousState().Gamepad.sThumbLX < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && (*controller)->GetPreviousState().Gamepad.sThumbLX > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) && (((*controller)->GetState().Gamepad.sThumbLX >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) || ((*controller)->GetState().Gamepad.sThumbLX <= -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE))) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " pressed LEFT-JOYSTICK X-AXIS" << endl;
			} else if ((((*controller)->GetPreviousState().Gamepad.sThumbLX >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) || ((*controller)->GetPreviousState().Gamepad.sThumbLX <= -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)) && (((*controller)->GetState().Gamepad.sThumbLX >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) || ((*controller)->GetState().Gamepad.sThumbLX <= -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE))) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " held LEFT-JOYSTICK X-AXIS" << endl;

				//Entity *boulder = EntityManager::FindEntities("Boulder")[0];
				//float x = -0.1f * (*controller)->GetState().Gamepad.sThumbLX / 30000.f;
				//boulder->transform.Rotate(Transform::UP, dt.GetTimeSeconds() * x);

				std::cout << (*controller)->GetState().Gamepad.sThumbLX << std::endl;

				vector<Component*> vehicleComponents = EntityManager::GetComponents(ComponentType_Vehicle);
				VehicleComponent* vehicle = static_cast<VehicleComponent*>(vehicleComponents[controllerNum]);
				vehicle->pxVehicleInputData.setAnalogSteer(-(*controller)->GetState().Gamepad.sThumbLX / 32768.0f);

			} else if (((*controller)->GetPreviousState().Gamepad.sThumbLX >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE || (*controller)->GetPreviousState().Gamepad.sThumbLX <= -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) && (((*controller)->GetState().Gamepad.sThumbLX < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) && ((*controller)->GetState().Gamepad.sThumbLX > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE))) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " released LEFT-JOYSTICK X-AXIS" << endl;

				vector<Component*> vehicleComponents = EntityManager::GetComponents(ComponentType_Vehicle);
				VehicleComponent* vehicle = static_cast<VehicleComponent*>(vehicleComponents[controllerNum]);
				vehicle->pxVehicleInputData.setAnalogSteer(0.0f);

			}

			//Left Joystick Y-Axis
			if (((*controller)->GetPreviousState().Gamepad.sThumbLY < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && (*controller)->GetPreviousState().Gamepad.sThumbLY > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) && (((*controller)->GetState().Gamepad.sThumbLY >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) || ((*controller)->GetState().Gamepad.sThumbLY <= -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE))) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " pressed LEFT-JOYSTICK Y-AXIS" << endl;
			} else if ((((*controller)->GetPreviousState().Gamepad.sThumbLY >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) || ((*controller)->GetPreviousState().Gamepad.sThumbLY <= -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)) && (((*controller)->GetState().Gamepad.sThumbLY >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) || ((*controller)->GetState().Gamepad.sThumbLY <= -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE))) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " held LEFT-JOYSTICK Y-AXIS" << endl;
			} else if (((*controller)->GetPreviousState().Gamepad.sThumbLY >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE || (*controller)->GetPreviousState().Gamepad.sThumbLY <= -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) && (((*controller)->GetState().Gamepad.sThumbLY < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) && ((*controller)->GetState().Gamepad.sThumbLY > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE))) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " released LEFT-JOYSTICK Y-AXIS" << endl;
			}
			
			//Right Joystick X-Axis
			if (((*controller)->GetPreviousState().Gamepad.sThumbRX < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE && (*controller)->GetPreviousState().Gamepad.sThumbRX > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) && (((*controller)->GetState().Gamepad.sThumbRX >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) || ((*controller)->GetState().Gamepad.sThumbRX <= -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE))) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " pressed RIGHT-JOYSTICK X-AXIS" << endl;
			} else if ((((*controller)->GetPreviousState().Gamepad.sThumbRX >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) || ((*controller)->GetPreviousState().Gamepad.sThumbRX <= -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)) && (((*controller)->GetState().Gamepad.sThumbRX >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) || ((*controller)->GetState().Gamepad.sThumbRX <= -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE))) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " held RIGHT-JOYSTICK X-AXIS" << endl;

				Entity *camera = EntityManager::FindEntities("Camera")[controllerNum];
				CameraComponent* cameraC = static_cast<CameraComponent*>(camera->components[0]);
				float x = dt.GetTimeSeconds() * 3.f * (*controller)->GetState().Gamepad.sThumbRX / 30000.f;
				cameraC->SetCameraHorizontalAngle(cameraC->GetCameraHorizontalAngle() + x);
				

			} else if (((*controller)->GetPreviousState().Gamepad.sThumbRX >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || (*controller)->GetPreviousState().Gamepad.sThumbRX <= -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) && (((*controller)->GetState().Gamepad.sThumbRX < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) && ((*controller)->GetState().Gamepad.sThumbRX > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE))) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " released RIGHT-JOYSTICK X-AXIS" << endl;
			}

			//Right Joystick Y-Axis
			if (((*controller)->GetPreviousState().Gamepad.sThumbRY < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE && (*controller)->GetPreviousState().Gamepad.sThumbRY > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) && (((*controller)->GetState().Gamepad.sThumbRY >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) || ((*controller)->GetState().Gamepad.sThumbRY <= -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE))) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " pressed RIGHT-JOYSTICK Y-AXIS" << endl;
			} else if ((((*controller)->GetPreviousState().Gamepad.sThumbRY >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) || ((*controller)->GetPreviousState().Gamepad.sThumbRY <= -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)) && (((*controller)->GetState().Gamepad.sThumbRY >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) || ((*controller)->GetState().Gamepad.sThumbRY <= -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE))) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " held RIGHT-JOYSTICK Y-AXIS" << endl;


				Entity *camera = EntityManager::FindEntities("Camera")[controllerNum];
				CameraComponent* cameraC = static_cast<CameraComponent*>(camera->components[0]);
				float x = dt.GetTimeSeconds() * 3.f * (*controller)->GetState().Gamepad.sThumbRY / 30000.f;
				cameraC->SetCameraVerticalAngle(std::min(std::max(cameraC->GetCameraVerticalAngle() + x,0.1f),static_cast<float>(M_PI) -0.1f));


			} else if (((*controller)->GetPreviousState().Gamepad.sThumbRY >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || (*controller)->GetPreviousState().Gamepad.sThumbRY <= -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) && (((*controller)->GetState().Gamepad.sThumbRY < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) && ((*controller)->GetState().Gamepad.sThumbRY > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE))) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " released RIGHT-JOYSTICK Y-AXIS" << endl;
			}

			// -------------------------------------------------------------------------------------------------------------- //
			// BUTTONS
			// -------------------------------------------------------------------------------------------------------------- //

			//Manage Button States
			int heldButtons = (*controller)->GetState().Gamepad.wButtons & (*controller)->GetPreviousState().Gamepad.wButtons;
			int pressedButtons = ((*controller)->GetState().Gamepad.wButtons ^ (*controller)->GetPreviousState().Gamepad.wButtons) & (*controller)->GetState().Gamepad.wButtons;
			int releasedButtons = ((*controller)->GetState().Gamepad.wButtons ^ (*controller)->GetPreviousState().Gamepad.wButtons) & (*controller)->GetPreviousState().Gamepad.wButtons;

			//DPAD-UP
			if (pressedButtons & XINPUT_GAMEPAD_DPAD_UP) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " pressed DPAD-UP" << endl;
			} else if (heldButtons & XINPUT_GAMEPAD_DPAD_UP) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " held DPAD-UP" << endl;
			} else if (releasedButtons & XINPUT_GAMEPAD_DPAD_UP) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " released DPAD-UP" << endl;
			}

			//DPAD-DOWN
			if (pressedButtons & XINPUT_GAMEPAD_DPAD_DOWN) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " pressed DPAD-DOWN" << endl;
			} else if (heldButtons & XINPUT_GAMEPAD_DPAD_DOWN) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " held DPAD-DOWN" << endl;
			} else if (releasedButtons & XINPUT_GAMEPAD_DPAD_DOWN) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " released DPAD-DOWN" << endl;
			}

			//DPAD-LEFT
			if (pressedButtons & XINPUT_GAMEPAD_DPAD_LEFT) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " pressed DPAD-LEFT" << endl;
			} else if (heldButtons & XINPUT_GAMEPAD_DPAD_LEFT) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " held DPAD-LEFT" << endl;
			} else if (releasedButtons & XINPUT_GAMEPAD_DPAD_LEFT) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " released DPAD-LEFT" << endl;
			}

			//DPAD-RIGHT
			if (pressedButtons & XINPUT_GAMEPAD_DPAD_RIGHT) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " pressed DPAD-RIGHT" << endl;
			} else if (heldButtons & XINPUT_GAMEPAD_DPAD_RIGHT) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " held DPAD-RIGHT" << endl;
			} else if (releasedButtons & XINPUT_GAMEPAD_DPAD_RIGHT) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " released DPAD-RIGHT" << endl;
			}

			//START
			if (pressedButtons & XINPUT_GAMEPAD_START) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " pressed START" << endl;
			} else if (heldButtons & XINPUT_GAMEPAD_START) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " held START" << endl;
			} else if (releasedButtons & XINPUT_GAMEPAD_START) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " released START" << endl;
			}

			//BACK
			if (pressedButtons & XINPUT_GAMEPAD_BACK) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " pressed BACK" << endl;
			} else if (heldButtons & XINPUT_GAMEPAD_BACK) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " held BACK" << endl;
			} else if (releasedButtons & XINPUT_GAMEPAD_BACK) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " released BACK" << endl;
			}

			//LEFT-THUMB
			if (pressedButtons & XINPUT_GAMEPAD_LEFT_THUMB) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " pressed LEFT-THUMB" << endl;
			} else if (heldButtons & XINPUT_GAMEPAD_LEFT_THUMB) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " held LEFT-THUMB" << endl;
			} else if (releasedButtons & XINPUT_GAMEPAD_LEFT_THUMB) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " released LEFT-THUMB" << endl;
			}

			//RIGHT-THUMB
			if (pressedButtons & XINPUT_GAMEPAD_RIGHT_THUMB) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " pressed RIGHT-THUMB" << endl;
			} else if (heldButtons & XINPUT_GAMEPAD_RIGHT_THUMB) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " held RIGHT-THUMB" << endl;
			} else if (releasedButtons & XINPUT_GAMEPAD_RIGHT_THUMB) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " released RIGHT-THUMB" << endl;
			}

			//LEFT-SHOULDER
			if (pressedButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " pressed LEFT-SHOULDER" << endl;
			} else if (heldButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " held LEFT-SHOULDER" << endl;
			} else if (releasedButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " released LEFT-SHOULDER" << endl;
			}

			//RIGHT-SHOULDER
			if (pressedButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " pressed RIGHT-SHOULDER" << endl;
			} else if (heldButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " held RIGHT-SHOULDER" << endl;
			} else if (releasedButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " released RIGHT-SHOULDER" << endl;
			}

			//A
			if (pressedButtons & XINPUT_GAMEPAD_A) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " pressed A" << endl;
			} else if (heldButtons & XINPUT_GAMEPAD_A) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " held A" << endl;
			} else if (releasedButtons & XINPUT_GAMEPAD_A) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " released A" << endl;
			}

			//B
			if (pressedButtons & XINPUT_GAMEPAD_B) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " pressed B" << endl;
			} else if (heldButtons & XINPUT_GAMEPAD_B) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " held B" << endl;
			} else if (releasedButtons & XINPUT_GAMEPAD_B) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " released B" << endl;
			}

			//X
			if (pressedButtons & XINPUT_GAMEPAD_X) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " pressed X" << endl;
			} else if (heldButtons & XINPUT_GAMEPAD_X) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " held X" << endl;
			} else if (releasedButtons & XINPUT_GAMEPAD_X) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " released X" << endl;
			}

			//Y
			if (pressedButtons & XINPUT_GAMEPAD_Y) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " pressed Y" << endl;
			} else if (heldButtons & XINPUT_GAMEPAD_Y) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " held Y" << endl;
			} else if (releasedButtons & XINPUT_GAMEPAD_Y) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " released Y" << endl;
			}

			//Vibrate Controller
			(*controller)->Vibrate(leftVibrate, rightVibrate);

			//Update Previous Controller State
			(*controller)->SetPreviousState((*controller)->GetState());
		}
	}
}