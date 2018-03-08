#pragma once
#include "System.h"
#include <vector>
#include <PxRigidDynamic.h>
#include "../Components/AiComponent.h"
#include "Content/NavigationMesh.h"

enum Map {
	Map_Cylinder = 0
};

enum GameMode {
	Team = 0,
	FreeForAll
};

// TODO: DriverType?

struct VehicleType {
    enum { Light = 0, Medium, Heavy, Count };
    static const std::string displayNames[Count];
    static const std::string prefabPaths[Count];
};

struct WeaponType {
    enum { MachineGun = 0, RocketLauncher, RailGun, Count };
    static const std::string displayNames[Count];
    static const std::string prefabPaths[Count];
    static const std::string turretPrefabPaths[Count];
};

struct PlayerData {
    PlayerData() : ready(false), vehicleType(0), weaponType(0), alive(false), vehicleEntity(nullptr) {}

    bool ready;
    int vehicleType;
    int weaponType;

    bool alive;
    Entity* vehicleEntity;
};

class Entity;
class Game : public System {
public:
    static const unsigned int MAX_VEHICLE_COUNT;

	// Access the singleton instance
	static Game& Instance();

	void Initialize();
	void Update() override;

    void InitializeGame();

	//Game Creation Variables
	static Map selectedMap;
	static GameMode selectedGameMode;
	static size_t numberOfAi;
	static size_t numberOfLives;
	static size_t killLimit;
	static size_t timeLimitMinutes;
	static size_t numberOfPlayers;
    static PlayerData players[4];

	std::string MapToString();
	std::string GameModeToString();

    NavigationMesh *GetNavigationMesh();

private:
	// No instantiation or copying
	Game();
	Game(const Game&) = delete;
	Game& operator= (const Game&) = delete;

	std::vector<Entity*> cameras;
	std::vector<Entity*> cars;

    physx::PxRigidDynamic *cylinderRigid;

    std::vector<Entity*> waypoints;

	Entity *boulder;
	Entity *camera;
	Entity *sun;
	Entity *floor;
	Entity *baby;

	Entity *car;

    NavigationMesh *navigationMesh;
};
