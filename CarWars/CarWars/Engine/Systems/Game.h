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

    size_t map;
    size_t gameMode;

    size_t playerCount;
    static constexpr size_t MIN_PLAYER_COUNT = 1;
    static constexpr size_t MAX_PLAYER_COUNT = 4;

    size_t aiCount;
    static constexpr size_t MIN_AI_COUNT = 0;
    static constexpr size_t MAX_AI_COUNT = 20;

    size_t numberOfLives;
    static constexpr size_t MIN_NUMBER_OF_LIVES = 1;
    static constexpr size_t MAX_NUMBER_OF_LIVES = 100;

    size_t killLimit;
    static constexpr size_t MIN_KILL_LIMIT = 1;
    static constexpr size_t MAX_KILL_LIMIT = 100;

    size_t timeLimitMinutes;
    static constexpr size_t MIN_TIME_LIMIT_MINUTES = 1;
    static constexpr size_t MAX_TIME_LIMIT_MINUTES = 60;
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
