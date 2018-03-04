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

void InputManager::HandleMouse() {
	//Mouse Inputs
	//Get Graphics Instance
	Graphics& graphicsInstance = Graphics::Instance();
	if (StateManager::GetState() == GameState_Playing) {
		//Get Vehicle Entity
		Entity* vehicle = EntityManager::FindEntities("Vehicle")[0];

		//Shoot Weapon
		if (Mouse::ButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
			static_cast<WeaponComponent*>(vehicle->components[1])->Charge();
		} else if (Mouse::ButtonDown(GLFW_MOUSE_BUTTON_LEFT)) {
			static_cast<WeaponComponent*>(vehicle->components[1])->Shoot();
		}

		//Cursor Inputs
		glfwSetInputMode(graphicsInstance.GetWindow(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		int width, height;
		glfwGetFramebufferSize(graphicsInstance.GetWindow(), &width, &height);
		double xPos, yPos;
		Mouse::GetCursorPosition(graphicsInstance.GetWindow(), &xPos, &yPos);

		//Get Camera Component
		CameraComponent* cameraComponent = static_cast<CameraComponent*>(EntityManager::GetComponents(ComponentType_Camera)[0]);
		//Get Weapon Child
		Entity* vehicleGunTurret = EntityManager::FindChildren(vehicle, "GunTurret")[0];
		glm::vec3 vehicleForward = vehicle->transform.GetForward();
		glm::vec3 vehicleUp = vehicle->transform.GetUp();
		float dotFR = glm::dot(vehicleForward, Transform::RIGHT);
		float dotUR = glm::dot(vehicleUp, Transform::RIGHT);
		float dotFF = glm::dot(vehicleForward, Transform::FORWARD);
		float dotUU = glm::dot(vehicleUp, Transform::UP);
		bool correctForward = dotFR > 0;
		bool correctUp = dotUR < 0;
		//Update Camera Angles
		float cameraHor = cameraComponent->GetCameraHorizontalAngle();
		float cameraVer = cameraComponent->GetCameraVerticalAngle();
		float cameraSpd = cameraComponent->GetCameraSpeed();
		float cursorHor = ((float)(width / 2.0f) - xPos);
		float cursorVer = ((float)(height / 2.0f) - yPos);
		cameraComponent->SetCameraHorizontalAngle(cameraHor - cursorHor * cameraSpd * StateManager::deltaTime.GetTimeSeconds());
		cameraComponent->SetCameraVerticalAngle(cameraVer + cursorVer * cameraSpd * StateManager::deltaTime.GetTimeSeconds());
		//Clamp Camera Angles
		float carAngleOffset = acos(glm::dot(vehicle->transform.GetUp(), Transform::UP));
		float minAngle = ((2.0f / 3.0f) * (M_PI_2)) + carAngleOffset * (correctUp ? 1.0f : -1.0f) * (correctForward ? -1.0f : 1.0f);
		float maxAngle = (float)(M_PI_2) + carAngleOffset * (correctUp ? 1.0f : -1.0f) * (correctForward ? -1.0f : 1.0f);
		if (cameraComponent->GetCameraVerticalAngle() < minAngle) {
			cameraComponent->SetCameraVerticalAngle(minAngle);
		} else if (cameraComponent->GetCameraVerticalAngle() > maxAngle) {
			cameraComponent->SetCameraVerticalAngle(maxAngle);
		}
		//Set Weapon Angle
		float gunHor = -cameraHor - M_PI_2 + acos(dotFF) * (correctForward ? 1.0f : -1.0f);
		vehicleGunTurret->transform.SetRotationAxisAngles(Transform::UP, gunHor);
		float gunVer = -cameraVer + acos(dotUU) * (correctForward ? -1.0f : 1.0f) * (correctUp ? 1.0f : -1.0f) + M_PI_2 - (M_PI_4 / 2.0f);
		vehicleGunTurret->transform.Rotate(Transform::RIGHT, gunVer);

		//Set Cursor to Middle
		glfwSetCursorPos(graphicsInstance.GetWindow(), width / 2, height / 2);
	} else {
		//Set Cursor Visible
		glfwSetInputMode(graphicsInstance.GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void InputManager::HandleKeyboard() {
	//Keyboard Inputs

    const GameState gameState = StateManager::GetState();

    // Navigate buttons up/down
    if (gameState >= GameState_Menu && gameState < __GameState_Menu_End) {
        std::string buttonGroupName;
        bool noNavigation = false;
        switch (gameState) {
        case GameState_Menu:
            buttonGroupName = "MainMenu_Buttons";
            break;
        case GameState_Menu_Start:
            buttonGroupName = "StartMenu_Buttons";
            break;
        default:
            noNavigation = true;
        }
        
        if (!noNavigation) {
            if (Keyboard::KeyPressed(GLFW_KEY_UP) || Keyboard::KeyPressed(GLFW_KEY_W)) GuiHelper::SelectPreviousGui(buttonGroupName);
            if (Keyboard::KeyPressed(GLFW_KEY_DOWN) || Keyboard::KeyPressed(GLFW_KEY_S)) GuiHelper::SelectNextGui(buttonGroupName);
        }
    }
    
    if (gameState == GameState_Menu) {
        if (Keyboard::KeyPressed(GLFW_KEY_ENTER)) {
            GuiComponent *selected = GuiHelper::GetSelectedGui("MainMenu_Buttons");
            if (selected->HasText("start")) {
                StateManager::SetState(GameState_Menu_Start);
            } else if (selected->HasText("options")) {
                StateManager::SetState(GameState_Menu_Settings);
            } else if (selected->HasText("exit")) {
                StateManager::SetState(GameState_Exit);
            }
        }
    } else if (gameState == GameState_Menu_Settings) {
        //Press Escape to Go Back a Screen
        if (Keyboard::KeyPressed(GLFW_KEY_ESCAPE)) {
            StateManager::SetState(GameState_Menu);
        }
    } else if (gameState == GameState_Menu_Start) {
        //Press Enter to Go to Confirm
        if (Keyboard::KeyPressed(GLFW_KEY_ENTER)) {
            GuiComponent *selected = GuiHelper::GetSelectedGui("StartMenu_Buttons");
            if (selected->HasText("back")) {
                StateManager::SetState(GameState_Menu);
            } else {
                StateManager::SetState(GameState_Menu_Start_CharacterSelect);
            }
        }
    } else if (gameState == GameState_Menu_Start_CharacterSelect) {
        if (Keyboard::KeyPressed(GLFW_KEY_ENTER)) {     // TODO: Add to controller controls (index per controller)
            GuiComponent *selected = GuiHelper::GetSelectedGui("CharacterMenu_Buttons");
            if (selected->HasText("a to join")) {
                selected->SetText("a to continue");
            } else {
                StateManager::SetState(GameState_Playing);
            }
        }
        //Return to Previous Screen
        if (Keyboard::KeyPressed(GLFW_KEY_ESCAPE)) {
            StateManager::SetState(GameState_Menu_Start);
        }
    }  else if (gameState == GameState_Playing) {
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
            StateManager::SetState(GameState_Paused);
            
        }
    } else if (gameState == GameState_Paused) {

		//Go to Game Playing
		if (Keyboard::KeyPressed(GLFW_KEY_ESCAPE)) {
			StateManager::SetState(GameState_Playing);
			
		}
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

				//std::cout << (*controller)->GetState().Gamepad.sThumbLX << std::endl;

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
				cameraC->SetCameraVerticalAngle(std::min(std::max(cameraC->GetCameraVerticalAngle() + x, 0.1f),static_cast<float>(M_PI)-0.1f));


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

                vector<Component*> vehicleComponents = EntityManager::GetComponents(ComponentType_Vehicle);
                VehicleComponent* vehicle = static_cast<VehicleComponent*>(vehicleComponents[controllerNum]);
                vehicle->pxVehicleInputData.setAnalogHandbrake(1.f);

			} else if (releasedButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) {
				cout << "Controller: " << (*controller)->GetControllerNumber() << " released LEFT-SHOULDER" << endl;

                vector<Component*> vehicleComponents = EntityManager::GetComponents(ComponentType_Vehicle);
                VehicleComponent* vehicle = static_cast<VehicleComponent*>(vehicleComponents[controllerNum]);
                vehicle->pxVehicleInputData.setAnalogHandbrake(0.f);
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
