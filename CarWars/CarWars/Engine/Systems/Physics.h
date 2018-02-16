#pragma once

#include "System.h"

#include "PxPhysicsAPI.h"
#include "Physics/VehicleSceneQuery.h"
#include "Physics/VehicleCreate.h"

class VehicleComponent;

class Physics : public System {
public:
	// Access the singleton instance
	static Physics& Instance();
    ~Physics();

    PxPhysics* GetApi() const;

    void Initialize();
    void InitializeVehicles();

	void Update(Time currentTime, Time deltaTime) override;

private:
	// No instantiation or copying
	Physics();
	Physics(const Physics&) = delete;
	Physics& operator= (const Physics&) = delete;

    static const PxVehiclePadSmoothingData gPadSmoothingData;
    static const PxVehicleKeySmoothingData gKeySmoothingData;
    static const PxF32 gSteerVsForwardSpeedData[2 * 8];
    static const PxFixedSizeLookupTable<8> gSteerVsForwardSpeedTable;

    physx::PxDefaultAllocator pxAllocator;
    physx::PxDefaultErrorCallback pxErrorCallback;

    physx::PxFoundation* pxFoundation = NULL;
    physx::PxPhysics* pxPhysics = NULL;
    
    physx::PxDefaultCpuDispatcher* pxDispatcher = NULL;
    physx::PxScene* pxScene = NULL;

    physx::PxCooking* pxCooking = NULL;

    physx::PxMaterial* pxMaterial = NULL;

    physx::PxPvd* pxPvd = NULL;

    VehicleSceneQueryData* pxVehicleSceneQueryData = NULL;
    physx::PxBatchQuery* pxBatchQuery = NULL;

    physx::PxVehicleDrivableSurfaceToTireFrictionPairs* pxFrictionPairs = NULL;

    physx::PxRigidStatic* pxGroundPlane = NULL;
};
