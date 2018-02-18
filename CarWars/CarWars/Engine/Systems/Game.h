#pragma once
#include "System.h"
#include <vector>

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
};
