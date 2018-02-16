#define _USE_MATH_DEFINES
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Engine/Systems/Graphics.h"
#include "Engine/Systems/InputManager.h"
#include "Engine/Systems/Game.h"
#include "Engine/Systems/Physics.h"
#include "Engine/Systems/Physics/CollisionFilterShader.h"
#include "Engine/Systems/Content/ContentManager.h"

using namespace std;

int main() {
	//Initialize Time Variables
	Time deltaTime;
	Time currentTime = 0;
	Time lastTime = 0;
	Time gameTime = 0;

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

    // Initialize game (MUST come before InitializeVehicles)
    Game &gameManager = Game::Instance();
    gameManager.Initialize();

    // Initialize vehicles
    physicsManager.InitializeVehicles();

	// Add systems in desired order
	systems.push_back(&inputManager);
	systems.push_back(&physicsManager);
	systems.push_back(&gameManager);
	systems.push_back(&graphicsManager);

	//Game Loop
	Time lastFrame(0);
	while (!glfwWindowShouldClose(graphicsManager.GetWindow())) {
		//Calculate Delta Time
		currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;
		//Calculate Game Time
		if (StateManager::IsState(GameState_Playing)) {
			gameTime += deltaTime;
		}
		// Iterate through each system and call their update methods
		for (System* system : systems) {
			system->Update(currentTime, deltaTime);
		}
	}
}
