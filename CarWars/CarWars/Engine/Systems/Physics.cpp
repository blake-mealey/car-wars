#include "Physics.h"

#include "PxPhysicsAPI.h"

#include "vehicle/PxVehicleUtil.h"

#include "../Entities/Entity.h"
#include "../Entities/EntityManager.h"
#include "../Entities/Transform.h"
#include "../Components/RigidbodyComponents/VehicleComponent.h"

#include "Physics/VehicleSceneQuery.h"
#include "Physics/VehicleTireFriction.h"
#include "Physics/VehicleCreate.h"
#include "Game.h"
#include "StateManager.h"
#include "Physics/CollisionGroups.h"
#include "Content/ContentManager.h"
#include <iostream>

using namespace std;

using namespace physx;

float nextTime = 0;
int i = 1;

// Singleton
Physics::Physics() { }
Physics &Physics::Instance() {
	static Physics instance;
	return instance;
}

Physics::~Physics() {
    /*gVehicle4W->getRigidDynamicActor()->release();        // TODO: VehicleComponent destructor
    gVehicle4W->free();*/
    pxBatchQuery->release();
    pxVehicleSceneQueryData->free(pxAllocator);
    pxFrictionPairs->release();
    PxCloseVehicleSDK();

    pxMaterial->release();
    pxCooking->release();
    pxScene->release();
    pxDispatcher->release();
    pxPhysics->release();
    PxPvdTransport* transport = pxPvd->getTransport();
    pxPvd->release();
    transport->release();
    pxFoundation->release();
}

PxPhysics& Physics::GetApi() const {
    return *pxPhysics;
}

PxCooking& Physics::GetCooking() const {
    return *pxCooking;
}

physx::PxScene& Physics::GetScene() const {
    return *pxScene;
}

const PxVehiclePadSmoothingData Physics::gPadSmoothingData =
{
    {
        6.0f,	//rise rate eANALOG_INPUT_ACCEL
        6.0f,	//rise rate eANALOG_INPUT_BRAKE		
        6.0f,	//rise rate eANALOG_INPUT_HANDBRAKE	
        2.5f,	//rise rate eANALOG_INPUT_STEER_LEFT
        2.5f,	//rise rate eANALOG_INPUT_STEER_RIGHT
    },
    {
        10.0f,	//fall rate eANALOG_INPUT_ACCEL
        10.0f,	//fall rate eANALOG_INPUT_BRAKE		
        10.0f,	//fall rate eANALOG_INPUT_HANDBRAKE	
        5.0f,	//fall rate eANALOG_INPUT_STEER_LEFT
        5.0f	//fall rate eANALOG_INPUT_STEER_RIGHT
    }
};

const PxVehicleKeySmoothingData Physics::gKeySmoothingData =
{
    {
        6.0f,	//rise rate eANALOG_INPUT_ACCEL
        6.0f,	//rise rate eANALOG_INPUT_BRAKE		
        6.0f,	//rise rate eANALOG_INPUT_HANDBRAKE	
        2.5f,	//rise rate eANALOG_INPUT_STEER_LEFT
        2.5f,	//rise rate eANALOG_INPUT_STEER_RIGHT
    },
    {
        10.0f,	//fall rate eANALOG_INPUT_ACCEL
        10.0f,	//fall rate eANALOG_INPUT_BRAKE		
        10.0f,	//fall rate eANALOG_INPUT_HANDBRAKE	
        5.0f,	//fall rate eANALOG_INPUT_STEER_LEFT
        5.0f	//fall rate eANALOG_INPUT_STEER_RIGHT
    }
};

const PxF32 Physics::gSteerVsForwardSpeedData[2 * 8] =
{
    0.0f,		0.75f,
    5.0f,		0.75f,
    30.0f,		0.125f,
    120.0f,		0.1f,
    PX_MAX_F32, PX_MAX_F32,
    PX_MAX_F32, PX_MAX_F32,
    PX_MAX_F32, PX_MAX_F32,
    PX_MAX_F32, PX_MAX_F32
};
const PxFixedSizeLookupTable<8> Physics::gSteerVsForwardSpeedTable(gSteerVsForwardSpeedData, 4);

void Physics::Initialize() {
    pxFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, pxAllocator, pxErrorCallback);
    pxPvd = PxCreatePvd(*pxFoundation);
    PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    pxPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
    pxPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *pxFoundation, PxTolerancesScale(), true, pxPvd);
    PxInitExtensions(*pxPhysics, pxPvd);

    PxSceneDesc sceneDesc(pxPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);

    const PxU32 numWorkers = 1;
    pxDispatcher = PxDefaultCpuDispatcherCreate(numWorkers);
    sceneDesc.cpuDispatcher = pxDispatcher;
    sceneDesc.filterShader = CollisionGroups::FilterShader;

    pxScene = pxPhysics->createScene(sceneDesc);
    pxScene->setFlag(PxSceneFlag::eENABLE_ACTIVE_ACTORS, true);
    PxPvdSceneClient* pvdClient = pxScene->getScenePvdClient();
    if (pvdClient)
    {
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }
    pxMaterial = ContentManager::GetPxMaterial("Default.json");

    pxCooking = PxCreateCooking(PX_PHYSICS_VERSION, *pxFoundation, PxCookingParams(PxTolerancesScale()));

    InitializeVehicles();
}

void Physics::InitializeVehicles() {
    PxInitVehicleSDK(*pxPhysics);
    PxVehicleSetBasisVectors(PxVec3(0, 1, 0), PxVec3(0, 0, 1));
    PxVehicleSetUpdateMode(PxVehicleUpdateMode::eVELOCITY_CHANGE);

    //Create the batched scene queries for the suspension raycasts.
    pxVehicleSceneQueryData = VehicleSceneQueryData::allocate(Game::MAX_VEHICLE_COUNT, PX_MAX_NB_WHEELS, 1, Game::MAX_VEHICLE_COUNT, WheelSceneQueryPreFilterBlocking, NULL, pxAllocator);
    pxBatchQuery = VehicleSceneQueryData::setUpBatchedSceneQuery(0, *pxVehicleSceneQueryData, pxScene);

    //Create the friction table for each combination of tire and surface type.
    pxFrictionPairs = createFrictionPairs(pxMaterial);
}

void Physics::Update() {
    if (StateManager::GetState() != GameState_Playing) return;

    const PxF32 timestep = 1.0f / 60.0f;

    //Raycasts.
    vector<Component*> vehicleComponents = EntityManager::GetComponents(ComponentType_Vehicle);
    vector<PxVehicleWheels*> vehicles;
    for (Component* component : vehicleComponents) {
        VehicleComponent* vehicle = static_cast<VehicleComponent*>(component);
        vehicles.push_back(vehicle->pxVehicle);

        // Update vehicle inputs
        if (vehicle->inputTypeDigital) {
            PxVehicleDrive4WSmoothDigitalRawInputsAndSetAnalogInputs(gKeySmoothingData, gSteerVsForwardSpeedTable, vehicle->pxVehicleInputData, timestep, vehicle->inAir, *vehicle->pxVehicle);
        } else {
            PxVehicleDrive4WSmoothAnalogRawInputsAndSetAnalogInputs(gPadSmoothingData, gSteerVsForwardSpeedTable, vehicle->pxVehicleInputData, timestep, vehicle->inAir, *vehicle->pxVehicle);
        }
    }

    PxRaycastQueryResult* raycastResults = pxVehicleSceneQueryData->getRaycastQueryResultBuffer(0);
    const PxU32 raycastResultsSize = pxVehicleSceneQueryData->getQueryResultBufferSize();
    PxVehicleSuspensionRaycasts(pxBatchQuery, vehicles.size(), vehicles.data(), raycastResultsSize, raycastResults);
    
    //Vehicle update.
    const PxVec3 grav = pxScene->getGravity();
    PxWheelQueryResult wheelQueryResults[PX_MAX_NB_WHEELS];
    vector<PxVehicleWheelQueryResult> vehicleQueryResults;
    for (PxVehicleWheels* vehicle : vehicles) {
        vehicleQueryResults.push_back({ wheelQueryResults, vehicle->mWheelsSimData.getNbWheels() });
    }
    PxVehicleUpdates(timestep, grav, *pxFrictionPairs, vehicles.size(), vehicles.data(), vehicleQueryResults.data());

    //Work out if the vehicle is in the air.
    for (Component* component : vehicleComponents) {
        VehicleComponent* vehicle = static_cast<VehicleComponent*>(component);
        vehicle->inAir = vehicle->pxVehicle->getRigidDynamicActor()->isSleeping() ? false : PxVehicleIsInAir(vehicleQueryResults[0]);
    }

    //Scene update.
    pxScene->simulate(timestep);
    pxScene->fetchResults(true);

    // Retrieve array of actors that moved
    PxU32 nbActiveActors;
    PxActor** activeActors = pxScene->getActiveActors(nbActiveActors);

    // Update each render object with the new transform
    for (PxU32 i = 0; i < nbActiveActors; ++i) {
        PxRigidActor* activeActor = static_cast<PxRigidActor*>(activeActors[i]);

        Component* component = static_cast<Component*>(activeActor->userData);
        if (component) component->UpdateFromPhysics(activeActor->getGlobalPose());
    }
}
