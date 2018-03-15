#pragma once
#include "IO/XboxController.h"
#include "IO/Keyboard.h"
#include "IO/Mouse.h"

#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <iostream>
using namespace std;

struct GuiNavData {
	GuiNavData() : vertDir(0), horizDir(0), enter(0), back(0), escape(0), tabPressed(0), tabHeld(0), tabReleased(0), playerIndex(0) {}

	int vertDir;
	int horizDir;
	int enter;
	int back;
	int escape;
	int tabPressed;
	int tabHeld;
	int tabReleased;
	int playerIndex;

	void NormalizeInputs() {
		const bool horizontal = abs(vertDir) < abs(horizDir);
		const bool vertical = abs(vertDir) > abs(horizDir);

		// Normalize inputs
		vertDir = vertical ? vertDir / abs(vertDir) : 0;
		horizDir = horizontal ? horizDir / abs(horizDir) : 0;
	}

	bool Valid() {
		return vertDir || horizDir || enter || back || escape || tabPressed || tabHeld || tabReleased;
	}
};

class InputManager : public System {
public:
	static InputManager& Instance();

	void Update() override;

	void HandleMouse();
	void HandleKeyboard();
	void HandleController();

    size_t GetControllerCount();
private:
	void NavigateGuis(GuiNavData navData);

	static vector<XboxController*> xboxControllers;

    void HandleVehicleControllerInput(size_t controllerNum, int &leftVibrate, int &rightVibrate);
};