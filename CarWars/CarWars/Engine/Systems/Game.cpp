#include "Game.h"

#include "Content/ContentManager.h"
#include "../Entities/EntityManager.h"
#include "../Components/SpotLightComponent.h"
#include "../Components/MeshComponent.h"
#include "../Components/CameraComponent.h"
#include "../Components/GuiComponents/GuiComponent.h"
#include "../Components/RigidbodyComponents/RigidStaticComponent.h"

#include "Physics\VehicleCreate.h"

#include <glm/gtx/string_cast.hpp>

#define _USE_MATH_DEFINES
#include <math.h>
#include "StateManager.h"
#include "../Components/DirectionLightComponent.h"
#include "../Components/RigidbodyComponents/RigidDynamicComponent.h"
#include "../Components/RigidbodyComponents/VehicleComponent.h"
#include "../Components/WeaponComponents/WeaponComponent.h"
#include "../Components/WeaponComponents/MachineGunComponent.h"
#include "../Components/WeaponComponents/RailGunComponent.h"
#include "../Components/WeaponComponents/RocketLauncherComponent.h"
#include "Physics.h"
#include "../Components/AiComponent.h"
#include "Pathfinder.h"
using namespace std;

const string GameModeType::displayNames[Count] = { "Team", "Free for All" };

const string MapType::displayNames[Count] = { "Cylinder" };
const string MapType::scenePaths[Count] = { "PhysicsDemo.json" };

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
    ContentManager::DestroySceneAndLoadScene("PhysicsDemo.json");

	for (int i = 0; i < gameData.playerCount; ++i) {
        PlayerData& player = players[i];

        player.vehicleEntity = ContentManager::LoadEntity(VehicleType::prefabPaths[player.vehicleType]);
        player.vehicleEntity->GetComponent<VehicleComponent>()->pxRigid->setGlobalPose(PxTransform(PxVec3(0.f, 10.f, i*15.f)));

        Entity* turret = ContentManager::LoadEntity(WeaponType::turretPrefabPaths[player.weaponType], player.vehicleEntity);
        turret->transform.SetPosition(EntityManager::FindFirstChild(player.vehicleEntity, "GunTurretBase")->transform.GetLocalPosition());

        Component* weapon = ContentManager::LoadComponent(WeaponType::prefabPaths[player.weaponType]);
        EntityManager::AddComponent(player.vehicleEntity, weapon);

        player.cameraEntity = ContentManager::LoadEntity("Game/Camera.json");
        
	    player.camera = player.cameraEntity->GetComponent<CameraComponent>();
        player.camera->SetCameraHorizontalAngle(-3.14 / 2);
        player.camera->SetCameraVerticalAngle(3.14 / 4);
        ContentManager::LoadScene("GUIs/HUD.json", player.camera->GetGuiRoot());
	}

    for (size_t i = 0; i < gameData.aiCount; ++i) {
        Entity *ai = ContentManager::LoadEntity("AiSewage.json");
        ai->GetComponent<VehicleComponent>()->pxRigid->setGlobalPose(PxTransform(PxVec3(15.f + 5.f * i, 10.f, 0.f)));

        Entity* turret = ContentManager::LoadEntity(WeaponType::turretPrefabPaths[WeaponType::MachineGun]);
        turret->transform.SetPosition(EntityManager::FindFirstChild(ai, "GunTurretBase")->transform.GetLocalPosition());
        EntityManager::SetParent(turret, ai);

		MachineGunComponent *gun = new MachineGunComponent();
		EntityManager::AddComponent(ai, gun);
    }

    Physics &physics = Physics::Instance();

    Entity *cylinder = EntityManager::FindEntities("Cylinder")[0];
    RigidDynamicComponent *cylinderRigid = cylinder->GetComponent<RigidDynamicComponent>();
    RigidStaticComponent *cylinderStatic = cylinder->GetComponent<RigidStaticComponent>();
    this->cylinderRigid = cylinderRigid->actor;

    // Don't let forces rotate the cylinder
    cylinderRigid->actor->setAngularDamping(0.f);
    cylinderRigid->actor->setMassSpaceInertiaTensor(PxVec3(0.f, 0.f, 0.f));

    // Lock the static rigidbody to the dynamic rigidbody so that the cylinder isn't affected
    // by forces in the scene
    PxFixedJoint *lock = PxFixedJointCreate(physics.GetApi(),
        cylinderStatic->pxRigid, PxTransform(PxIdentity),
        cylinderRigid->actor, PxTransform(PxIdentity));

    // Enable visual debugging for constraints
    physics.GetScene().setVisualizationParameter(PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1.0f);
    physics.GetScene().setVisualizationParameter(PxVisualizationParameter::eJOINT_LIMITS, 1.0f);
    lock->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);


    waypoints = EntityManager::FindEntities("Waypoint");

    navigationMesh = new NavigationMesh({
        { "ColumnCount", 100 },
        { "RowCount", 100 },
        { "Spacing", 2.5f }
    });

	std::vector<AiComponent*> ais = EntityManager::GetComponents<AiComponent>(ComponentType_AI);
    for (AiComponent* ai : ais) {
        switch (ai->GetMode()) {
        case AiMode_Waypoints:
            ai->SetTargetEntity(waypoints[0]);
            break;
        case AiMode_Chase:
            ai->SetTargetEntity(EntityManager::FindEntities("Vehicle")[0]);
            break;
        }
        ai->UpdatePath();
    }
}

void Game::Update() {
    if (StateManager::GetState() < __GameState_Menu_End) {
        for (Entity* entity : EntityManager::FindEntities("VehicleBox")) {
            entity->transform.Rotate(Transform::UP, 0.005f);
        }
        if (StateManager::GetState() == GameState_Menu_Settings || StateManager::GetState() == GameState_Menu_Start) {
            CameraComponent* camera = EntityManager::FindEntities("Camera")[0]->GetComponent<CameraComponent>();
            const double tick = StateManager::globalTime.GetTimeSeconds() / 10.f;
            camera->SetPosition(100.f * glm::vec3(cos(tick), 0.f, sin(tick)));
        }
    } else if (StateManager::GetState() == GameState_Playing) {
        // Set the cylinder's rotation
        cylinderRigid->setAngularVelocity(PxVec3(0.f, 0.f, 0.06f));

        // Update AIs
		std::vector<Component*> aiComponents = EntityManager::GetComponents(ComponentType_AI);
        for (Component *component : aiComponents) {
			AiComponent *ai = static_cast<AiComponent*>(component);
            if (!ai->enabled) continue;
            VehicleComponent* vehicle = ai->GetEntity()->GetComponent<VehicleComponent>();

            Transform &myTransform = ai->GetEntity()->transform;
            const glm::vec3 position = myTransform.GetGlobalPosition();
            const glm::vec3 forward = myTransform.GetForward();
            const glm::vec3 right = myTransform.GetRight();

            ai->UpdatePath();       // Will only update every x seconds
            const glm::vec3 targetPosition = ai->GetTargetEntity()->transform.GetGlobalPosition();
            const glm::vec3 nodePosition = ai->NodeInPath();

            glm::vec3 direction = nodePosition - position;
            const float distance = glm::length(direction);
            direction = glm::normalize(direction);

            if (distance <= navigationMesh->GetSpacing() * 2.f) {
                ai->NextNodeInPath();
            }

            switch(ai->GetMode()) {
            case AiMode_Waypoints:
                if (glm::length(targetPosition - position) <= navigationMesh->GetSpacing()) {
                    ai->SetTargetEntity(waypoints[ai->NextWaypoint(4)]);
                }
            case AiMode_Chase:
                const float steer = glm::dot(direction, right);
                const PxReal speed = vehicle->pxVehicle->computeForwardSpeed();

                if (!ai->IsStuck() && abs(speed) <= 0.5f) {
                    ai->SetStuck(true);
                } else if (ai->IsStuck() && abs(speed) >= 1.f) {
                    ai->SetStuck(false);
                }

                if (!ai->IsReversing() && ai->IsStuck() && ai->GetStuckDuration().GetTimeSeconds() >= 1.f) {
                    ai->StartReversing();
                }

                if (ai->IsReversing() && ai->GetReversingDuration().GetTimeSeconds() >= 2.f) {
                    ai->StopReversing();
                }

                const bool reverse = ai->IsReversing();// speed < 1.f; // glm::dot(direction, forward) > -0.1;

				const float accel = glm::clamp(distance / 20.f, 0.1f, 0.8f) * reverse ? 0.8f : 0.8f;

                if (!reverse && vehicle->pxVehicle->mDriveDynData.getCurrentGear() == PxVehicleGearsData::eREVERSE) {
                    vehicle->pxVehicle->mDriveDynData.forceGearChange(PxVehicleGearsData::eFIRST);
                } else if (reverse && vehicle->pxVehicle->mDriveDynData.getCurrentGear() != PxVehicleGearsData::eREVERSE) {
                    vehicle->pxVehicle->mDriveDynData.forceGearChange(PxVehicleGearsData::eREVERSE);
                }

                vehicle->pxVehicleInputData.setAnalogSteer(reverse ? -steer : steer);
                vehicle->pxVehicleInputData.setAnalogAccel(accel);

                break;
            }

            if (ai->FinishedPath()) {
                ai->UpdatePath();
            }
        }

        // Update sun direction
		const float t = glm::radians(45.5) + StateManager::gameTime.GetTimeSeconds() / 10;
        const glm::vec3 sunPosition = glm::vec3(cos(t), 0.5f, sin(t));
        EntityManager::FindEntities("Sun")[0]->GetComponent<DirectionLightComponent>()->SetDirection(-sunPosition);
		
        // Update player cameras
        for (int i = 0; i < gameData.playerCount; ++i) {
            PlayerData& player = players[i];
            player.cameraEntity->transform.SetPosition(EntityManager::FindChildren(player.vehicleEntity, "GunTurret")[0]->transform.GetGlobalPosition());
            player.camera->SetTarget(player.vehicleEntity->transform.GetGlobalPosition() +
                player.vehicleEntity->transform.GetUp() * 2.f + player.vehicleEntity->transform.GetForward() * -1.25f);
        }
	} else if (StateManager::GetState() == GameState_Paused) {

        // PAUSED

	}
}

NavigationMesh* Game::GetNavigationMesh() {
    return navigationMesh;
}
