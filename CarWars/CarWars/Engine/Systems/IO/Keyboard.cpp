#include "Keyboard.h"

bool Keyboard::keyDown[GLFW_KEY_LAST] = { false };
bool Keyboard::keyPressed[GLFW_KEY_LAST] = { false };
bool Keyboard::keyReleased[GLFW_KEY_LAST] = { false };

/*
Callback to update keys being pressed
*/
void Keyboard::KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key < 0) {
		return;
	}

	if (action == GLFW_PRESS && keyDown[key] == false) {
		keyPressed[key] = true;
		keyReleased[key] = false;
	}

	if (action == GLFW_RELEASE && keyDown[key] == true) {
		keyPressed[key] = false;
		keyReleased[key] = true;
	}

	keyDown[key] = action != GLFW_RELEASE;
}

/*
Returns whether a given key is held down
*/
bool Keyboard::KeyDown(int key) {
	return keyDown[key];
}

/*
Returns whether a given key was pressed in the most recent frame
*/
bool Keyboard::KeyPressed(int key) {
	bool tmp = keyPressed[key];
	keyPressed[key] = false;
	return tmp;
}

/*
Returns whether a given key was released in the most recent frame
*/
bool Keyboard::KeyReleased(int key) {
	bool tmp = keyReleased[key];
	keyReleased[key] = false;
	return tmp;
}