#include "Game.h"

#include "Content/ContentManager.h"
#include "../Entities/EntityManager.h"
#include "../Components/SpotLightComponent.h"
#include "../Components/MeshComponent.h"
#include "../Components/CameraComponent.h"
#include "../Components/GuiComponents/GuiComponent.h"
#include "../Components/RigidbodyComponents/RigidStaticComponent.h"

#include <glm/gtx/string_cast.hpp>

#define _USE_MATH_DEFINES
#include <math.h>
#include "StateManager.h"
#include "../Components/DirectionLightComponent.h"
#include "../Components/RigidbodyComponents/VehicleComponent.h"
#include "../Components/WeaponComponents/WeaponComponent.h"
#include "Physics.h"
#include "../Components/AiComponent.h"
#include "Pathfinder.h"
#include "../Components/GuiComponents/GuiHelper.h"
using namespace std;

const string GameModeType::displayNames[Count] = { "Team", "Free for All" };

const string MapType::displayNames[Count] = { "Circle" };
const string MapType::scenePaths[Count] = { "Maps/CircleMap.json" };

const string VehicleType::displayNames[Count] = { "Heavy", "Medium", "Light" };
const string VehicleType::prefabPaths[Count] = { "Vehicles/Sewage.json", "Vehicles/Hearse.json", "Vehicles/Flatbed.json" };
const string VehicleType::statDisplayNames[STAT_COUNT] = { "Speed", "Handling", "Armour" };
const string VehicleType::statValues[Count][STAT_COUNT] = {
	{ "1", "10", "100" },      // Heavy
	{ "10", "10", "50" },      // Medium
	{ "20", "10", "10" }       // Light
};

const string WeaponType::displayNames[Count] = { "Machine Gun", "Rocket Launcher", "Rail Gun" };
const string WeaponType::prefabPaths[Count] = { "Weapons/MachineGun.json", "Weapons/RocketLauncher.json", "Weapons/RailGun.json" };
const string WeaponType::turretPrefabPaths[Count] = { "Weapons/MachineGunTurret.json", "Weapons/RocketLauncherTurret.json", "Weapons/RailGunTurret.json" };
const string WeaponType::statDisplayNames[STAT_COUNT] = {"rof", "Damage", "Type"};
const string WeaponType::statValues[Count][STAT_COUNT] = {
	{ "100", "1", "bullet" },      // Machine Gun
	{ "50", "50", "rocket" },      // Rocket Launcher
	{ "1", "100", "charge" }       // Rail Gun
};

const unsigned int Game::MAX_VEHICLE_COUNT = 20;

GameData Game::gameData;
PlayerData Game::players[4];
vector<AiData> Game::ais;

Time gameTime(0);

// Singleton
Game::Game() {}
Game &Game::Instance() {
	static Game instance;
	return instance;
}

float unitRand() {
	return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

void Game::Initialize() {
    ContentManager::LoadSkybox("PurpleNebula/");
    
    StateManager::SetState(GameState_Menu);
}

void Game::InitializeGame() {
    // Initialize the map
    ContentManager::DestroySceneAndLoadScene(MapType::scenePaths[gameData.map]);

    // Initialize game stuff
    gameData.timeLimit = Time::FromMinutes(gameData.timeLimitMinutes);

    // Initialize teams
    size_t teamCount = 0;
    if (gameData.gameMode == GameModeType::Team) teamCount = 2;
    else if (gameData.gameMode == GameModeType::FreeForAll) teamCount = gameData.playerCount + gameData.aiCount;
    for (size_t i = 0; i < teamCount; ++i) {
        TeamData team;
        if (gameData.gameMode == GameModeType::Team) {
            team.name = "Team " + to_string(i + 1);
        }
        gameData.teams.push_back(team);
    }

    // Initialize the players
	for (int i = 0; i < gameData.playerCount; ++i) {
        PlayerData& player = players[i];
		player.name = "Player " + to_string(i + 1);
        player.alive = true;
		player.follow = false;

        // Set their team
        if (gameData.gameMode == GameModeType::FreeForAll) {
            player.teamIndex = i;
            gameData.teams[player.teamIndex].name = player.name;
        } else if (gameData.gameMode == GameModeType::Team) {
            player.teamIndex = i % 2;
        }

        // Initialize their vehicle
        // TODO: Proper spawn location
        player.vehicleEntity = ContentManager::LoadEntity(VehicleType::prefabPaths[player.vehicleType]);
        player.vehicleEntity->GetComponent<VehicleComponent>()->pxRigid->setGlobalPose(PxTransform(PxVec3(0.f, 10.f, i*15.f)));

        // Initialize their turret mesh
        Entity* turret = ContentManager::LoadEntity(WeaponType::turretPrefabPaths[player.weaponType], player.vehicleEntity);
        turret->transform.SetPosition(EntityManager::FindFirstChild(player.vehicleEntity, "GunTurretBase")->transform.GetLocalPosition());

        // Initialize their weapon
        Component* weapon = ContentManager::LoadComponent(WeaponType::prefabPaths[player.weaponType]);
        EntityManager::AddComponent(player.vehicleEntity, weapon);

        // Initialize their camera
        player.cameraEntity = ContentManager::LoadEntity("Game/Camera.json");
	    player.camera = player.cameraEntity->GetComponent<CameraComponent>();
        player.camera->SetCameraHorizontalAngle(-3.14 / 2);
        player.camera->SetCameraVerticalAngle(3.14 / 4);

        // Initialize their UI
        ContentManager::LoadScene("GUIs/HUD.json", player.camera->GetGuiRoot());
	}

    // Initialize the AI
    for (size_t i = 0; i < gameData.aiCount; ++i) {
        // Create the AI
        // TODO: Choose vehicle and weapon type somehow
        ais.push_back(AiData(VehicleType::Heavy, WeaponType::MachineGun));
        AiData& ai = ais[i];
		ai.alive = true;
		ai.diffuculty = 1.f;
		ai.name = "Computer " + to_string(i + 1);

        // Set their team
        if (gameData.gameMode == GameModeType::FreeForAll) {
            ai.teamIndex = gameData.playerCount + i;
            gameData.teams[ai.teamIndex].name = ai.name;
        } else if (gameData.gameMode == GameModeType::Team) {
            ai.teamIndex = (gameData.playerCount + i) % 2;
        }

        // Initialize their vehicle
        // TODO: Proper spawn location
        ai.vehicleEntity = ContentManager::LoadEntity(VehicleType::prefabPaths[ai.vehicleType]);
        ai.vehicleEntity->GetComponent<VehicleComponent>()->pxRigid->setGlobalPose(PxTransform(PxVec3(15.f + 5.f * i, 10.f, 0.f)));

        // Initialize their turret mesh
        Entity* turret = ContentManager::LoadEntity(WeaponType::turretPrefabPaths[WeaponType::MachineGun], ai.vehicleEntity);
        turret->transform.SetPosition(EntityManager::FindFirstChild(ai.vehicleEntity, "GunTurretBase")->transform.GetLocalPosition());

        // Initialize their weapon
        Component* weapon = ContentManager::LoadComponent(WeaponType::prefabPaths[ai.weaponType]);
        EntityManager::AddComponent(ai.vehicleEntity, weapon);

        // Initialize their AI
        ai.brain = static_cast<AiComponent*>(ContentManager::LoadComponent("Ai.json"));
        EntityManager::AddComponent(ai.vehicleEntity, ai.brain);
    }

    waypoints = EntityManager::FindEntities("Waypoint");

    navigationMesh = new NavigationMesh({
        { "ColumnCount", 100 },
        { "RowCount", 100 },
        { "Spacing", 2.5f }
    });

	std::vector<AiComponent*> ais = EntityManager::GetComponents<AiComponent>(ComponentType_AI);
    for (AiComponent* ai : ais) {
		ai->SetMode();
    }
}

void ResetVehicleData(VehicleData& vehicle) {
    vehicle.alive = false;
    vehicle.killCount = 0;
    vehicle.deathCount = 0;
}

void Game::ResetGame() {
    // Reset players
    for (size_t i = 0; i < gameData.playerCount; ++i) {
        PlayerData& player = players[i];
        player.ready = false;
        ResetVehicleData(player);
    }

    // Reset ais
    for (AiData& ai : ais) {
        ResetVehicleData(ai);
    }

    // Reset game
    gameData.playerCount = 0;
    gameData.teams.clear();
    StateManager::gameTime = 0.0;
}

void Game::FinishGame() {
    StateManager::SetState(GameState_Menu_GameEnd);
}

void Game::Update() {
    if (StateManager::GetState() < __GameState_Menu_End) {
        for (Entity* entity : EntityManager::FindEntities("VehicleBox")) {
            entity->transform.Rotate(Transform::UP, 0.005f);
        }
        if (StateManager::GetState() == GameState_Menu_Settings || StateManager::GetState() == GameState_Menu_Start) {
            CameraComponent* camera = EntityManager::FindEntities("Camera")[0]->GetComponent<CameraComponent>();
            const double tick = StateManager::globalTime.GetSeconds() / 10.f;
            camera->SetPosition(100.f * glm::vec3(cos(tick), 0.f, sin(tick)));
        }
    } else if (StateManager::GetState() == GameState_Playing) {

        // Update AIs
		std::vector<Component*> aiComponents = EntityManager::GetComponents(ComponentType_AI);
		for (Component *component : aiComponents) {
			AiComponent *ai = static_cast<AiComponent*>(component);
			ai->Update();
		}

        // Update sun direction
		const float t = glm::radians(45.5) + StateManager::gameTime.GetSeconds() / 10;
        const glm::vec3 sunPosition = glm::vec3(cos(t), 0.5f, sin(t));
        EntityManager::FindEntities("Sun")[0]->GetComponent<DirectionLightComponent>()->SetDirection(-sunPosition);
		
        // Update player cameras
        for (int i = 0; i < gameData.playerCount; ++i) {
            PlayerData& player = players[i];
            if (!player.alive) continue;
            player.cameraEntity->transform.SetPosition(EntityManager::FindChildren(player.vehicleEntity, "GunTurret")[0]->transform.GetGlobalPosition());
			player.camera->SetTarget(player.vehicleEntity->transform.GetGlobalPosition());
			
			PxScene* scene = &Physics::Instance().GetScene();
			PxRaycastBuffer hit;
			glm::vec3 direction = glm::normalize(player.camera->GetPosition() - player.camera->GetTarget());
			player.camera->SetTargetOffset(glm::vec3(0, 2, 0) + EntityManager::FindChildren(player.vehicleEntity, "GunTurret")[0]->transform.GetGlobalPosition() - player.vehicleEntity->transform.GetGlobalPosition());
			PxQueryFilterData filterData;
			filterData.data.word0 = -1 ^ player.vehicleEntity->GetComponent<VehicleComponent>()->GetRaycastGroup();
			//Raycast
			if (scene->raycast(Transform::ToPx(player.camera->GetTarget()), Transform::ToPx(direction), CameraComponent::MAX_DISTANCE + 1, hit, PxHitFlag::eDEFAULT, filterData)) {
				player.camera->SetDistance(hit.block.distance - .5);
			}
			else {
				player.camera->SetDistance(CameraComponent::MAX_DISTANCE);
			}
        }

        // ---------------
        // Gamemode update
        // ---------------

        // Update clock and score UIs
        size_t highestTeamKillCount = 0;
        for (TeamData& team : gameData.teams) {
            if (team.killCount > highestTeamKillCount) highestTeamKillCount = team.killCount;
        }

        for (size_t i = 0; i < gameData.playerCount; ++i) {
            PlayerData& player = players[i];
            GuiHelper::SetFirstGuiText("GameClock", (gameData.timeLimit - StateManager::gameTime).ToString(), i);
            if (gameData.gameMode == GameModeType::Team) {
                GuiHelper::SetFirstGuiText("GameScores", to_string(gameData.teams[0].killCount), i);
                GuiHelper::SetSecondGuiText("GameScores", to_string(gameData.teams[1].killCount), i);
            } else if (gameData.gameMode == GameModeType::FreeForAll) {
                GuiHelper::SetFirstGuiText("GameScores", to_string(gameData.teams[player.teamIndex].killCount), i);
                GuiHelper::SetSecondGuiText("GameScores", to_string(highestTeamKillCount), i);
            }
        }

        // TODO: AI for kill limit and lives

        // Time limit
        if (StateManager::gameTime >= gameData.timeLimit) FinishGame();

        // Kill limit and lives
        bool allDeadForever = true;
        for (size_t i = 0; i < gameData.playerCount; ++i) {
            PlayerData& player = players[i];
            if (player.killCount >= gameData.killLimit) FinishGame();
            if (allDeadForever && player.deathCount < gameData.numberOfLives) allDeadForever = false;
        }
        if (allDeadForever) FinishGame();
	} else if (StateManager::GetState() == GameState_Paused) {

        // PAUSED

	}
}

NavigationMesh* Game::GetNavigationMesh() const {
    return navigationMesh;
}

VehicleData* Game::GetDataFromEntity(Entity* vehicle) {
    PlayerData* playerData = GetPlayerFromEntity(vehicle);
    if (playerData) return playerData;

    for (AiData& ai : ais) {
        if (ai.vehicleEntity == vehicle) return &ai;
    }
    return nullptr;
}

PlayerData* Game::GetPlayerFromEntity(Entity* vehicle) {
    for (size_t i = 0; i < gameData.playerCount; ++i) {
        PlayerData& player = players[i];
        if (player.vehicleEntity == vehicle) return &player;
    }
    return nullptr;
}
