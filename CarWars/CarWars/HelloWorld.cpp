#define _USE_MATH_DEFINES
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Engine/Systems/Graphics.h"
#include "Engine/Systems/InputManager.h"
#include "Engine/Systems/Game.h"
#include "Engine/Systems/Physics.h"
#include "Engine/Systems/Audio.h"
#include "Engine/Systems/Physics/CollisionGroups.h"
#include "Engine/Systems/Content/ContentManager.h"

using namespace std;

int main() {
	//Initialize Time Variables
	Time currentTime = 0;
	Time lastTime = 0;

	//Declare System Vector
	vector<System*> systems;

	// Initialize systems
	// Initialize graphics (MUST come before Game)
	Graphics &graphicsManager = Graphics::Instance();
	graphicsManager.Initialize("Car Wars");
	
	// Initialize input
	InputManager &inputManager = InputManager::Instance();

    // Initialize physics (MUST come before Game)
    Physics &physicsManager = Physics::Instance();
    physicsManager.Initialize();

    // Load and initialize collision groups (MUST come before Game)
    ContentManager::LoadCollisionGroups("Vehicles.json");
    CollisionGroups::InitializeMasks();     // MUST come after all collision groups have been loaded

    // Initialize game
    Game &gameManager = Game::Instance();
    gameManager.Initialize();

    // Initialize audio
    Audio &audioManager = Audio::Instance();
    audioManager.Initialize();

    //audioManager.PlayAudio("Content/Sounds/engine-running.mp3");

	// Add systems in desired order
	systems.push_back(&inputManager);
	systems.push_back(&physicsManager);
	systems.push_back(&gameManager);
	systems.push_back(&graphicsManager);
    systems.push_back(&audioManager);

	//Game Loop
	Time lastFrame(0);
	while (!glfwWindowShouldClose(graphicsManager.GetWindow())) {
		//Calculate Delta Time
		currentTime = glfwGetTime();
		StateManager::deltaTime = currentTime - lastTime;
		lastTime = currentTime;
		//Calculate Game Time
		if (StateManager::IsState(GameState_Playing)) {
			StateManager::gameTime += StateManager::deltaTime;
		}
		// Iterate through each system and call their update methods
		for (System* system : systems) {
			system->Update();
		}
	}
}
