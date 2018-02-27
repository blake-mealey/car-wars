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
private:
	static vector<XboxController*> xboxControllers;
};