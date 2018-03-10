#pragma once
#include "System.h"
#include <vector>
#include <PxRigidDynamic.h>
#include "Content/NavigationMesh.h"

class CameraComponent;

struct GameModeType {
    enum { Team = 0, FreeForAll, Count };
    static const std::string displayNames[Count];
};

struct MapType {
    enum { Cylinder = 0, Count };
    static const std::string displayNames[Count];
    static const std::string scenePaths[Count];
};

// TODO: DriverType?

struct VehicleType {
    enum { Light = 0, Medium, Heavy, Count };
    static const std::string displayNames[Count];
    static const std::string prefabPaths[Count];
	static constexpr size_t STAT_COUNT = 3;
	static const std::string statDisplayNames[STAT_COUNT];
	static const std::string statValues[Count][STAT_COUNT];		// accel, handle, resist
};

struct WeaponType {
    enum { MachineGun = 0, RocketLauncher, RailGun, Count };
    static const std::string displayNames[Count];
    static const std::string prefabPaths[Count];
    static const std::string turretPrefabPaths[Count];
	static constexpr size_t STAT_COUNT = 3;
	static const std::string statDisplayNames[STAT_COUNT];
	static const std::string statValues[Count][STAT_COUNT];		// rof, damage, type
};

struct PlayerData {
    PlayerData() : ready(false), vehicleType(0), weaponType(0),
        alive(false), vehicleEntity(nullptr), cameraEntity(nullptr), camera(nullptr) {}

	// Menu state
    bool ready;

	// Settings
    int vehicleType;
    int weaponType;

	// Game state
    bool alive;
    Entity* vehicleEntity;
    Entity* cameraEntity;
    CameraComponent* camera;
};

struct GameData {
    GameData() : map(0), gameMode(0), playerCount(0), aiCount(5),
        numberOfLives(3), killLimit(10), timeLimitMinutes(10) {}

    int map;
    int gameMode;
    size_t playerCount;
    size_t aiCount;
    size_t numberOfLives;
    size_t killLimit;
    size_t timeLimitMinutes;
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
    static GameData gameData;
    static PlayerData players[4];

    NavigationMesh *GetNavigationMesh();

private:
	// No instantiation or copying
	Game();
	Game(const Game&) = delete;
	Game& operator= (const Game&) = delete;

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
