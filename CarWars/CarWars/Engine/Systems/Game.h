#pragma once
#include "System.h"
#include <vector>
#include "Content/NavigationMesh.h"
#include "../Components/PowerUpComponents/PowerUp.h"
#include "Content/HeightMap.h"
#include "Content/Map.h"

class CameraComponent;
class AiComponent;
class SuicideWeaponComponent;

struct GameModeType {
    enum { Team = 0, FreeForAll, Count };
    static const std::string displayNames[Count];
};

struct MapType {
    enum { TestArena = 0, Moon, Arena, Tiers, BattleArena, BattleDome, Mansion, CircleArena, Levels, Count };
    static const std::string displayNames[Count];
    static const std::string mapDirPaths[Count];
};

// TODO: DriverType?

struct VehicleType {
    enum { Heavy = 0, Medium, Light, Count };
    static const std::string displayNames[Count];
    static const std::string prefabPaths[Count];
    static const std::string teamTextureNames[Count][2];
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
    static const std::string texturePaths[Count];
};

struct TeamData {
    TeamData() : killCount(0), deathCount(0), size(0) {}

    int killCount;
	int deathCount;
	size_t size;
    std::string name;
    size_t index;
};

struct PlayerData {
    PlayerData(int _vehicleType = VehicleType::Heavy, int _weaponType = WeaponType::MachineGun) :
        name(""), vehicleType(_vehicleType), weaponType(_weaponType),
        alive(false), vehicleEntity(nullptr),
        teamIndex(0), killCount(0), deathCount(0), activePowerUp(nullptr) {
	
		static int nextId = 0;
		id = nextId++;
	}

	int id;

    // Settings
    int vehicleType;
    int weaponType;

    // Game state
	Time diedTime;
    bool alive;
    Entity* vehicleEntity;

    // Gamemode state
	std::string name;
    size_t teamIndex;
    int killCount;
    int deathCount;
    PowerUp* activePowerUp;

	// For leaderboard sorting
	bool operator <(const PlayerData& rhs) const {
		return killCount > rhs.killCount;
	}

};

struct HumanData : PlayerData {
    HumanData() : PlayerData(), ready(false), cameraEntity(nullptr), camera(nullptr) {}

	// Menu state
    bool ready;
    Entity* cameraEntity;
    CameraComponent* camera;
};

struct AiData : PlayerData {
    AiData(int _vehicleType, int _weaponType, float _difficulty) : PlayerData(_vehicleType, _weaponType), brain(nullptr), difficulty(_difficulty){}

    // Game state
	float difficulty;
    AiComponent* brain;
};

struct GameData {
	GameData() : map(0), gameMode(0), humanCount(0), aiCount(1),
		numberOfLives(3), killLimit(10), timeLimitMinutes(10), aiDifficulty(5) {}

    size_t map;
    size_t gameMode;

    size_t humanCount;
    static constexpr size_t MIN_PLAYER_COUNT = 1;
    static constexpr size_t MAX_PLAYER_COUNT = 4;

    size_t aiCount;
    static constexpr size_t MIN_AI_COUNT = 0;
    static constexpr size_t MAX_AI_COUNT = 20;

    int numberOfLives;
    static constexpr size_t MIN_NUMBER_OF_LIVES = 1;
    static constexpr size_t MAX_NUMBER_OF_LIVES = 100;

    int killLimit;
    static constexpr size_t MIN_KILL_LIMIT = 1;
    static constexpr size_t MAX_KILL_LIMIT = 100;

    size_t timeLimitMinutes;
    Time timeLimit;
    static constexpr size_t MIN_TIME_LIMIT_MINUTES = 1;
    static constexpr size_t MAX_TIME_LIMIT_MINUTES = 60;

	size_t aiDifficulty;
	static constexpr size_t MIN_AI_DIFFICULTY = 0;
	static constexpr size_t MAX_AI_DIFFICULTY = 10.f;

	Time respawnTime = 1.0;

    std::vector<TeamData> teams;
};

class Entity;
class Game : public System {
public:
    static const unsigned int MAX_VEHICLE_COUNT;

	// Access the singleton instance
	static Game& Instance();

	void Initialize();
	void Update() override;

	void SpawnVehicle(PlayerData& vehicle) const;
	void SpawnAi(AiData& ai);
    void InitializeGame();
    void ResetGame();
    void FinishGame();

	//Game Creation Variables
    static GameData gameData;
    static HumanData humanPlayers[4];
    static vector<AiData> aiPlayers;

    NavigationMesh *GetNavigationMesh() const;
    HeightMap* GetHeightMap() const;

    static PlayerData* GetPlayerFromEntity(Entity* vehicle);
    static HumanData* GetHumanFromEntity(Entity* vehicle);
private:
	// No instantiation or copying
	Game();
	Game(const Game&) = delete;
	Game& operator= (const Game&) = delete;

    Map* map;

	SuicideWeaponComponent* suicide;
};
