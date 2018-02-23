#include "Mouse.h"

#include <iostream>

bool Mouse::buttonsDown[GLFW_MOUSE_BUTTON_LAST] = { false };
bool Mouse::buttonsPressed[GLFW_MOUSE_BUTTON_LAST] = { false };
bool Mouse::buttonsReleased[GLFW_MOUSE_BUTTON_LAST] = { false };

/*
Callback to update the mouse buttons being pressed
*/
void Mouse::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button < 0) {
		return;
	}

	if (action == GLFW_PRESS && buttonsDown[button] == false) {
		buttonsPressed[button] = true;
		buttonsReleased[button] = false;
	}

	if (action == GLFW_RELEASE && buttonsDown[button] == true) {
		buttonsDown[button] = false;
		buttonsReleased[button] = true;
	}

	buttonsDown[button] = action != GLFW_RELEASE;
}

void Mouse::GetCursorPosition(GLFWwindow* _window, double* _x, double* _y) {
	glfwGetCursorPos(_window, _x, _y);
}

/*
Returns whether a given button is held down
*/
bool Mouse::ButtonDown(int button) {
	return buttonsDown[button];
}

/*
Returns whether a given button was pressed in the most recent frame
*/
bool Mouse::ButtonPressed(int button) {
	bool tmp = buttonsPressed[button];
	buttonsPressed[button] = false;
	return tmp;
}

/*
Returns whether a given button was released in the most recent frame
*/
bool Mouse::ButtonReleased(int button) {
	bool tmp = buttonsReleased[button];
	buttonsReleased[button] = false;
	return tmp;
}