#pragma once
#include "IO/XboxController.h"
#include "IO/Keyboard.h"
#include "IO/Mouse.h"

#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <iostream>
using namespace std;

class InputManager : public System {
public:
	static InputManager& Instance();

	void Update() override;

	void HandleMouse();
	void HandleKeyboard();
	void HandleController();

    size_t GetControllerCount();
private:
	void NavigateGuis(int vertDir, int horizDir, int enter, int back, int escape, int playerIndex);

	static vector<XboxController*> xboxControllers;

    void HandleVehicleControllerInput(size_t controllerNum, int &leftVibrate, int &rightVibrate);
};