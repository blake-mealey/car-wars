#pragma once
#include "System.h"
#include <vector>

enum Maps {
	Map_Cylinder
};

enum GameMode {
	Team,
	FFA
};

class Entity;
class Game : public System {
public:
    static const unsigned int MAX_VEHICLE_COUNT;

	// Access the singleton instance
	static Game& Instance();

	void Initialize();
	void Update(Time currentTime, Time deltaTime) override;

private:
	// No instantiation or copying
	Game();
	Game(const Game&) = delete;
	Game& operator= (const Game&) = delete;

	std::vector<Entity*> cameras;
	std::vector<Entity*> cars;

	Entity *boulder;
	Entity *camera;
	Entity *sun;
	Entity *floor;
	Entity *baby;

	Entity *car;

	//Variables for Game Initialization
	Maps selectedMap;				//Map to Play on
	GameMode selectedGameMode;		//Game Mode to Play
	size_t numberAiVehicles;		//Number of AI Vehicles (Start)
	size_t startNumberLives;		//Starting Number of Lives
	size_t killLimit;				//Max Kills to Win
	Time timeLimit;					//Max Time of Game

	//Game Variables
	size_t numberVehiclesRemaining;
};
