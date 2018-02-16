#pragma once
#include "IO/XboxController.h"
#include "IO/Keyboard.h"
#include "IO/Mouse.h"

#include <vector>
#include <iostream>
using namespace std;

class InputManager : public System {
public:
	static InputManager& Instance();

	void Update(Time currentTime, Time deltaTime) override;

	void HandleMouse();
	void HandleKeyboard();
	void HandleController();
private:
	static vector<XboxController*> xboxControllers;
};