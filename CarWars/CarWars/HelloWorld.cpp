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
#include "Engine/Systems/Effects.h"

using namespace std;

int main() {
	//Declare System Vector
	vector<System*> systems;

	// Initialize systems
	// Initialize graphics (MUST come before Game)
	Graphics &graphicsManager = Graphics::Instance();
	graphicsManager.Initialize("Car Wars");

    Effects &guiEffectsManager = Effects::Instance();
	
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
    //audioManager.PlayAudio2D("Content/Music/unity.mp3");


	// Add systems in desired order
	systems.push_back(&inputManager);
	systems.push_back(&physicsManager);
	systems.push_back(&gameManager);
	systems.push_back(&guiEffectsManager);
	systems.push_back(&graphicsManager);
    systems.push_back(&audioManager);

    // Define the fixed physics time step
    constexpr double physicsTimeStep = 1.0 / 60.0;
    Time physicsTime;

	//Game Loop
	while (!glfwWindowShouldClose(graphicsManager.GetWindow())) {
		//Calculate Delta Time
        const Time lastTime = StateManager::globalTime;
        StateManager::globalTime = glfwGetTime();
		StateManager::deltaTime = StateManager::globalTime - lastTime;

		//Calculate Game Time
		if (StateManager::IsState(GameState_Playing)) {
			StateManager::gameTime += StateManager::deltaTime;
		}

		// Iterate through each system and call their update methods
        for (System* system : systems) {
            if (system != &physicsManager) {
                system->Update();
            } else {
                while (physicsTime < StateManager::globalTime) {
                    physicsTime += physicsTimeStep;
                    system->Update();
                }
            }
		}
	}
}
