#pragma once

#include "../StateManager.h"
#include "../../Entities/EntityManager.h"
#include "../../Components/CameraComponent.h"
#include "../../Components/WeaponComponents/MachineGunComponent.h"

#include <algorithm>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

class Mouse {
public:
	static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

	static void GetCursorPosition(GLFWwindow* _window, double* _x, double* _y);

	static bool ButtonDown(int button);
	static bool ButtonPressed(int button);
	static bool ButtonReleased(int button);

private:
	static bool buttonsDown[];
	static bool buttonsPressed[];
	static bool buttonsReleased[];
};