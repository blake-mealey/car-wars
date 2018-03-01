#include "Game.h"

#include "Content/ContentManager.h"
#include "../Entities/EntityManager.h"
#include "../Components/SpotLightComponent.h"
#include "../Components/MeshComponent.h"
#include "../Components/CameraComponent.h"
#include "../Components/RigidbodyComponents/RigidStaticComponent.h"

#include "Physics\VehicleCreate.h"

#include <glm/gtx/string_cast.hpp>

#define _USE_MATH_DEFINES
#include <math.h>
#include "StateManager.h"
#include "../Components/DirectionLightComponent.h"
#include "../Components/RigidbodyComponents/RigidDynamicComponent.h"
#include "../Components/RigidbodyComponents/VehicleComponent.h"
#include "Physics.h"
#include "../Components/AiComponent.h"
#include "Pathfinder.h"
using namespace std;

const unsigned int Game::MAX_VEHICLE_COUNT = 20;

Map Game::selectedMap = Map_Cylinder;
GameMode Game::selectedGameMode = Team;
size_t Game::numberOfAi = 0;
size_t Game::numberOfLives = 3;
size_t Game::killLimit = 10;
size_t Game::timeLimitMinutes = 10;

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

	ContentManager::LoadScene("PhysicsDemo.json");

    for (size_t i = 0; i < 1; ++i) {
        Entity *ai = ContentManager::LoadEntity("AiSewage.json");
        static_cast<VehicleComponent*>(ai->components[2])->pxRigid->setGlobalPose(PxTransform(PxVec3(15.f + 5.f * i, 10.f, 0.f)));
    }

	cars = EntityManager::FindEntities("Vehicle");
	cameras = EntityManager::FindEntities("Camera");

	for (Entity* camera : cameras) {
        static_cast<CameraComponent*>(camera->components[0])->SetCameraHorizontalAngle(-3.14 / 2);
        static_cast<CameraComponent*>(camera->components[0])->SetCameraVerticalAngle(3.14 / 4);
	}


    Physics &physics = Physics::Instance();

    Entity *cylinder = EntityManager::FindEntities("Cylinder")[0];
    RigidDynamicComponent *cylinderRigid = static_cast<RigidDynamicComponent*>(cylinder->components[1]);
    RigidStaticComponent *cylinderStatic = static_cast<RigidStaticComponent*>(cylinder->components[2]);
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


    ais = EntityManager::GetComponents<AiComponent>(ComponentType_AI);
    waypoints = EntityManager::FindEntities("Waypoint");

    navigationMesh = new NavigationMesh({
        { "ColumnCount", 50 },
        { "RowCount", 50 },
        { "Spacing", 5.f }
    });

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
    

	// Load the scene and get some entities
	/*ContentManager::LoadScene("GraphicsDemo.json");
	boulder = EntityManager::FindEntities("Boulder")[0];
	sun = EntityManager::FindEntities("Sun")[0];
	floor = EntityManager::FindEntities("Floor")[0];
	baby = EntityManager::FindEntities("Baby")[0];




	//camera->transform.SetPosition(glm::vec3(0, 5, 10));

	const int lightCount = 5;
	for (int i = 0; i < lightCount; ++i) {
		Entity *entity = EntityManager::CreateStaticEntity();

		const float angle = i * ((2 * M_PI) / lightCount);
		const glm::vec3 position = 6.f * glm::vec3(sin(angle), 0, cos(angle));
		entity->transform.SetPosition(position);
		entity->transform.SetScale(glm::vec3(0.1f));

		const glm::vec3 color = glm::vec3(unitRand(), unitRand(), unitRand());

		//PointLightComponent *light = new PointLightComponent(color, 10);
		SpotLightComponent *light = new SpotLightComponent(color, 20, glm::radians(20.f), -position - glm::vec3(0, 2, 0));
		EntityManager::AddComponent(entity, light);

		MeshComponent *mesh = new MeshComponent("Sphere.obj", new Material(color, color, 1));
		EntityManager::AddComponent(entity, mesh);
	}*/
}

void Game::Update() {
	if (StateManager::GetState() == GameState_Playing) {

		//boulder->transform.Translate(glm::vec3(0.0f, sin(currentTime.GetTimeSeconds()), 0.0f));
	    //const glm::vec3 pos = boulder->transform.GetLocalPosition();
        //boulder->transform.SetPosition(glm::vec3(pos.x, sin(gameTime.GetTimeMilliSeconds() / 500), pos.z));
		//boulder->transform.Rotate(glm::vec3(0, 1, 0), deltaTime.GetTimeMilliSeconds() * 0.00002);

		//boulder->transform.Translate(boulder->transform.GetForward() * 0.1f);

		//camera->transform.SetPosition(10.f * glm::vec3(
			//sin(gameTime.GetTimeMilliSeconds() / 1000), 0.5,
			//cos(gameTime.GetTimeMilliSeconds() / 1000)));

		//Rotate The Weapon along the horizonal
		//Entity *camera = EntityManager::FindEntities("Camera")[0];
		//CameraComponent* cameraC = static_cast<CameraComponent*>(camera->components[0]);
		//MachineGunComponent* gun = static_cast<MachineGunComponent*>(EntityManager::FindEntities("Vehicle")[0]->components[1]);
		//gun->SetTargetRotation(cameraC->GetCameraHorizontalAngle(), cameraC->GetCameraVerticalAngle());
		//Entity* vehicle = EntityManager::FindEntities("Vehicle")[0];
		//EntityManager::FindChildren(vehicle, "MachineGunTurret")[0]->transform.SetRotationAxisAngles(glm::vec3(0.0f, 1.0f, 0.0f), gun->horizontalAngle + glm::radians(90.0f));


        // Set the cylinder's rotation
        cylinderRigid->setAngularVelocity(PxVec3(0.f, 0.f, 0.06f));

        // Update AIs
        for (AiComponent *ai : ais) {
            if (!ai->enabled) continue;
            VehicleComponent* vehicle = static_cast<VehicleComponent*>(ai->GetEntity()->components[2]);

            Transform &myTransform = ai->GetEntity()->transform;
            const glm::vec3 position = myTransform.GetGlobalPosition();
            const glm::vec3 forward = myTransform.GetForward();
            const glm::vec3 right = myTransform.GetRight();

            const glm::vec3 targetPosition = ai->NodeInPath();

            glm::vec3 direction = targetPosition - position;
            const float distance = glm::length(direction);
            direction = glm::normalize(direction);

            if (distance <= navigationMesh->GetSpacing()) {
                ai->NextNodeInPath();
            }

            switch(ai->GetMode()) {
            case AiMode_Waypoints:
                if (ai->FinishedPath()) {
                    ai->SetTargetEntity(waypoints[ai->NextWaypoint(4)]);
                }
            case AiMode_Chase:
                const float steer = glm::dot(direction, right);
                const bool reverse = false; // glm::dot(direction, forward) > -0.1;

                const float accel = glm::clamp(distance / 20.f, 0.1f, 0.8f) * reverse ? 0.8f : 0.5f;

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


		float t = glm::radians(45.5) + gameTime.GetTimeSeconds() / 10;
        glm::vec3 sunPosition = glm::vec3(cos(t), 0.5f, sin(t));
        static_cast<DirectionLightComponent*>(EntityManager::FindEntities("Sun")[0]->components[0])->SetDirection(-sunPosition);
		
		for (int i = 0; i < cars.size(); i++) {
			Entity* camera = cameras[i];
			Entity* car = cars[i];

			//"Camera Delay"
			//camera->transform.SetPosition(glm::mix(camera->transform.GetGlobalPosition(), car->transform.GetGlobalPosition(), 0.04f));
			camera->transform.SetPosition(car->transform.GetGlobalPosition());
			static_cast<CameraComponent*>(camera->components[0])->SetTarget(car->transform.GetGlobalPosition() + car->transform.GetUp() * 2.f + car->transform.GetForward() * -1.f);
		}

		//camera->transform.SetPosition(boulder->transform.GetGlobalPosition());

		//floor->transform.Rotate({ 0,0,1 }, deltaTime.GetTimeMilliSeconds() * 0.00002);
		//baby->transform.Translate(glm::vec3(.01, 0, 0));
		//baby->transform.Rotate(glm::vec3(1,0,0),.01f);

	} else if (StateManager::GetState() == GameState_Paused) {

        // PAUSED

	}
}

std::string Game::MapToString() {
	switch (selectedMap) {
	case 0:
		return "Map_Cylinder";
	}
}

std::string Game::GameModeToString() {
	switch (selectedGameMode) {
	case 0:
		return "Team";
	case 1:
		return "FFA";
	}
}

NavigationMesh* Game::GetNavigationMesh() {
    return navigationMesh;
}
