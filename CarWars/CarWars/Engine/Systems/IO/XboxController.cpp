#include "XboxController.h"

XboxController::XboxController(int _playerNumber) {
	controllerNumber = _playerNumber - 1;
	SetPreviousState(GetState());
}

XINPUT_STATE XboxController::GetState() {
	//Zeroise the state
	ZeroMemory(&controllerState, sizeof(XINPUT_STATE));

	//Get the state
	XInputGetState(controllerNumber, &controllerState);

	return controllerState;
}

void XboxController::SetPreviousState(XINPUT_STATE _previousState) {
	previousControllerState = _previousState;
}

XINPUT_STATE XboxController::GetPreviousState() {
	return previousControllerState;
}

bool XboxController::IsConnected() {
	//Zeroise the state
	ZeroMemory(&controllerState, sizeof(XINPUT_STATE));

	//Get the state
	DWORD result = XInputGetState(controllerNumber, &controllerState);

	if (result == ERROR_SUCCESS) {
		return true;
	}
	else {
		return false;
	}
}

int XboxController::GetControllerNumber() {
	return controllerNumber;
}

void XboxController::Vibrate(int _leftVal, int _rightVal) {
	//Create Vibration State
	XINPUT_VIBRATION vibration;

	//Zeroise the Vibration
	ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));

	vibration.wLeftMotorSpeed = _leftVal;
	vibration.wRightMotorSpeed = _rightVal;

	//Vibrate the Controller
	XInputSetState(controllerNumber, &vibration);
}