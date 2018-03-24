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
#include "Effects.h"
#include "../Components/RigidbodyComponents/PowerUpSpawnerComponent.h"
using namespace std;

const string GameModeType::displayNames[Count] = { "Team", "Free for All" };

const string MapType::displayNames[Count] = { "Circle" };
const string MapType::scenePaths[Count] = { "Maps/HeightMap.json" };

const string VehicleType::displayNames[Count] = { "Heavy", "Medium", "Light" };
const string VehicleType::prefabPaths[Count] = { "Vehicles/Sewage.json", "Vehicles/Hearse.json", "Vehicles/Flatbed.json" };
const string VehicleType::teamTextureNames[Count][2] = {
    { "Vehicles/Green_Sewage.png", "Vehicles/Red_Sewage.png" },
    { "Vehicles/Green_Hearse.png", "Vehicles/Red_Hearse.png" },
    { "Vehicles/Green_Flatbed.png", "Vehicles/Red_Flatbed.png" }
};
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
HumanData Game::humanPlayers[4];
vector<AiData> Game::ais;

Time gameTime(0);

// Singleton
Game::Game(): heightMap(nullptr), navigationMesh(nullptr) {}

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

void Game::SpawnVehicle(PlayerData& player) const {
	vector<Entity*> spawns = EntityManager::FindEntities("SpawnLocation");
	Entity* spawn = spawns[rand() % spawns.size()];
	const glm::vec3 position = spawn->transform.GetGlobalPosition() + glm::vec3(0.f, 5.f, 0.f);

	// Initialize their vehicle
	player.vehicleEntity = ContentManager::LoadEntity(VehicleType::prefabPaths[player.vehicleType]);
	player.vehicleEntity->GetComponent<VehicleComponent>()->pxRigid->setGlobalPose(PxTransform(Transform::ToPx(position)));

    if (gameData.gameMode == GameModeType::Team) {
        MeshComponent* mesh = player.vehicleEntity->GetComponent<MeshComponent>();
        mesh->SetTexture(ContentManager::GetTexture(VehicleType::teamTextureNames[player.vehicleType][player.teamIndex]));
    }

	// Initialize their turret mesh
	Entity* turret = ContentManager::LoadEntity(WeaponType::turretPrefabPaths[player.weaponType], player.vehicleEntity);
	turret->transform.SetPosition(EntityManager::FindFirstChild(player.vehicleEntity, "GunTurretBase")->transform.GetLocalPosition());

	// Initialize their weapon
	Component* weapon = ContentManager::LoadComponent(WeaponType::prefabPaths[player.weaponType]);
	EntityManager::AddComponent(player.vehicleEntity, weapon);

	player.alive = true;
}

void Game::SpawnAi(AiData& ai) {
	SpawnVehicle(ai);
	
	// Initialize their AI
	ai.brain = static_cast<AiComponent*>(ContentManager::LoadComponent("Ai.json"));
	EntityManager::AddComponent(ai.vehicleEntity, ai.brain);
}

void Game::InitializeGame() {
    // Initialize the map
    ContentManager::ResetLastAccessedHeightMap();
    ContentManager::DestroySceneAndLoadScene(MapType::scenePaths[gameData.map]);
    heightMap = ContentManager::GetLastAccessedHeightMap();

    // Initialize game stuff
    gameData.timeLimit = Time::FromMinutes(gameData.timeLimitMinutes);

    // Initialize teams
    size_t teamCount = 0;
    if (gameData.gameMode == GameModeType::Team) teamCount = 2;
    else if (gameData.gameMode == GameModeType::FreeForAll) teamCount = gameData.humanCount + gameData.aiCount;
    for (size_t i = 0; i < teamCount; ++i) {
        TeamData team;
        if (gameData.gameMode == GameModeType::Team) {
            team.name = "Team " + to_string(i + 1);
        }
        gameData.teams.push_back(team);
    }

    // Initialize the humanPlayers
	for (int i = 0; i < gameData.humanCount; ++i) {
        HumanData& player = humanPlayers[i];
		player.name = "Player " + to_string(i + 1);

        // Set their team
        if (gameData.gameMode == GameModeType::FreeForAll) {
            player.teamIndex = i;
            gameData.teams[player.teamIndex].name = player.name;
        } else if (gameData.gameMode == GameModeType::Team) {
            player.teamIndex = i % 2;
        }

		SpawnVehicle(player);

        // Initialize their camera
        player.cameraEntity = ContentManager::LoadEntity("Game/Camera.json");
	    player.camera = player.cameraEntity->GetComponent<CameraComponent>();

		player.camera->SetTarget(EntityManager::FindChildren(player.vehicleEntity, "GunTurret")[0]->transform.GetGlobalPosition());
		player.camera->SetTargetOffset(glm::vec3(0, 2, 0));

		glm::vec3 vehicleDirection = player.vehicleEntity->transform.GetForward();
		vehicleDirection.y = 0;
		vehicleDirection = glm::normalize(vehicleDirection);

		player.camera->SetCameraHorizontalAngle(-player.camera->GetCameraHorizontalAngle() + acos(glm::dot(vehicleDirection, Transform::FORWARD)) * (glm::dot(vehicleDirection, Transform::RIGHT) > 0 ? 1 : -1) + M_PI_2);
		player.camera->SetCameraVerticalAngle(-player.camera->GetCameraVerticalAngle() + M_PI * .45f);
		player.camera->UpdatePositionFromAngles();
		
		// Initialize their UI
        ContentManager::LoadScene("GUIs/HUD.json", player.camera->GetGuiRoot());
	}

    // Initialize the AI
    for (size_t i = 0; i < gameData.aiCount; ++i) {
        // Create the AI
        // TODO: Choose vehicle and weapon type somehow

        ais.push_back(AiData(VehicleType::Heavy, WeaponType::MachineGun, gameData.aiDifficulty));
        AiData& ai = ais[i];
		ai.name = "Computer " + to_string(i + 1);

        // Set their team
        if (gameData.gameMode == GameModeType::FreeForAll) {
            ai.teamIndex = gameData.humanCount + i;
            gameData.teams[ai.teamIndex].name = ai.name;
        } else if (gameData.gameMode == GameModeType::Team) {
            ai.teamIndex = (gameData.humanCount + i) % 2;
        }

		SpawnAi(ai);
    }

    if (navigationMesh) delete navigationMesh;
    if (heightMap) {
        navigationMesh = new NavigationMesh({
            { "ColumnCount", heightMap->GetLength() / 2.5f },
            { "RowCount", heightMap->GetWidth() / 2.5f },
            { "Spacing", 2.5f }
        });
    } else {
        navigationMesh = new NavigationMesh({
            { "ColumnCount", 100 },
            { "RowCount", 100 },
            { "Spacing", 2.5f }
        });
    }
}

void ResetPlayerData(PlayerData& player) {
    player.alive = false;
    player.killCount = 0;
    player.deathCount = 0;
}

void Game::ResetGame() {
    // Reset humanPlayers
    for (size_t i = 0; i < gameData.humanCount; ++i) {
        HumanData& player = humanPlayers[i];
        player.ready = false;
        ResetPlayerData(player);
    }

    // Reset ais
    ais.clear();

    // Reset game
    gameData.humanCount = 0;
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
		/*const float t = glm::radians(45.5) + StateManager::gameTime.GetSeconds() / 10;
        const glm::vec3 sunPosition = glm::vec3(cos(t), 0.5f, sin(t));
        EntityManager::FindEntities("Sun")[0]->GetComponent<DirectionLightComponent>()->SetDirection(-sunPosition);*/
		
        // Update cameras
        for (int i = 0; i < gameData.humanCount; ++i) {
            HumanData& player = humanPlayers[i];
            if (!player.alive) continue;
            player.cameraEntity->transform.SetPosition(EntityManager::FindChildren(player.vehicleEntity, "GunTurret")[0]->transform.GetGlobalPosition());
			player.camera->SetTarget(EntityManager::FindChildren(player.vehicleEntity, "GunTurret")[0]->transform.GetGlobalPosition());
			player.camera->SetTargetOffset(glm::vec3(0, 2, 0));

			PxScene* scene = &Physics::Instance().GetScene();
			PxRaycastBuffer hit;
			glm::vec3 direction = glm::normalize(player.camera->GetPosition() - player.camera->GetTarget());
			PxQueryFilterData filterData;
			filterData.data.word0 = -1 ^ player.vehicleEntity->GetComponent<VehicleComponent>()->GetRaycastGroup();
			
			//Raycast
			if (scene->raycast(Transform::ToPx(player.camera->GetTarget()), Transform::ToPx(direction), CameraComponent::MAX_DISTANCE + 3, hit, PxHitFlag::eDEFAULT, filterData)) {
				player.camera->SetDistance(hit.block.distance - 3);
			} else {
				player.camera->SetDistance(CameraComponent::MAX_DISTANCE);
			}
        }

		// Respawn vehicles
		for (size_t i = 0; i < gameData.humanCount; ++i) {
			HumanData& player = humanPlayers[i];
			if (!player.alive && StateManager::gameTime >= player.diedTime + gameData.respawnTime && player.deathCount < gameData.numberOfLives) {
				SpawnVehicle(player);
                GuiComponent* gui = GuiHelper::GetSecondGui("HealthBar", i);
				gui->GetMask().SetScale(gui->transform.GetLocalScale());
			}
		}

		for (AiData& player : ais) {
			if (!player.alive && StateManager::gameTime >= player.diedTime + gameData.respawnTime && player.deathCount < gameData.numberOfLives) {
				SpawnAi(player);
			}
		}

        // Respawn powerups
        std::vector<Component*> powerUpSpawners = EntityManager::GetComponents(ComponentType_PowerUpSpawner);
        for (Component* component : powerUpSpawners) {
            PowerUpSpawnerComponent* spawner = static_cast<PowerUpSpawnerComponent*>(component);
            spawner->Respawn();
        }

        // Remove powerups from players
        for (size_t i = 0; i < gameData.humanCount; ++i) {
            HumanData& player = humanPlayers[i];
            if (player.activePowerUp) player.activePowerUp->Remove();
        }
        for (AiData& player : ais) if (player.activePowerUp) player.activePowerUp->Remove();

        // ---------------
        // Gamemode update
        // ---------------

        // Update clock and score UIs
        size_t highestTeamKillCount = 0;
        for (TeamData& team : gameData.teams) {
            if (team.killCount > highestTeamKillCount) highestTeamKillCount = team.killCount;
			if (team.killCount >= gameData.killLimit) {
				FinishGame();
				return;
			}
        }

        for (size_t i = 0; i < gameData.humanCount; ++i) {
            HumanData& player = humanPlayers[i];
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

        // Kill limit
        for (TeamData& team : gameData.teams) {
            if (team.killCount >= gameData.killLimit) FinishGame();
        }

        // Max lives
        size_t deadForeverCount = 0;
        for (size_t i = 0; i < gameData.humanCount; ++i) {
            PlayerData& player = humanPlayers[i];
            if (player.deathCount >= gameData.numberOfLives) deadForeverCount++;
        }
		for (size_t i = 0; i < gameData.aiCount; ++i) {
			PlayerData& player = ais[i];
			if (player.deathCount >= gameData.numberOfLives) deadForeverCount++;
		}
        if (deadForeverCount == gameData.aiCount + gameData.humanCount - 1) FinishGame();
	} else if (StateManager::GetState() == GameState_Paused) {
        // PAUSED
	}
}

NavigationMesh* Game::GetNavigationMesh() const {
    return navigationMesh;
}

HeightMap* Game::GetHeightMap() const {
    return heightMap;
}

PlayerData* Game::GetPlayerFromEntity(Entity* vehicle) {
    HumanData* player = GetHumanFromEntity(vehicle);
    if (player) return player;

    for (AiData& ai : ais) {
        if (ai.vehicleEntity == vehicle) return &ai;
    }
    return nullptr;
}

HumanData* Game::GetHumanFromEntity(Entity* vehicle) {
    for (size_t i = 0; i < gameData.humanCount; ++i) {
        HumanData& player = humanPlayers[i];
        if (player.vehicleEntity == vehicle) return &player;
    }
    return nullptr;
}
