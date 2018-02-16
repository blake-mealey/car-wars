#pragma once
#include <Windows.h>
#include <Xinput.h>
#pragma comment (lib, "XInput.lib")

class XboxController {
public:
	XboxController(int _playerNumber);

	XINPUT_STATE GetState();
	void SetPreviousState(XINPUT_STATE _previousState);
	XINPUT_STATE GetPreviousState();

	bool IsConnected();
	int GetControllerNumber();

	void Vibrate(int _leftVal = 0, int _rightVal = 0);

private:
	XINPUT_STATE controllerState;
	XINPUT_STATE previousControllerState;

	int controllerNumber;
};