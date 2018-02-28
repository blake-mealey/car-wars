#pragma once

#include <vector>

#include <vehicle/PxVehicleDrive4W.h>
#include <vehicle/PxVehicleUtilControl.h>
#include <json/json.hpp>
#include "RigidDynamicComponent.h"
#include "../MeshComponent.h"

struct AxleData {
    AxleData(const float _centerOffset = 0.f, const float _wheelInset = 0.f)
        : centerOffset(_centerOffset), wheelInset(_wheelInset) {};
    float centerOffset;
    float wheelInset;
};

class VehicleComponent : public RigidDynamicComponent {
public:
    VehicleComponent(nlohmann::json data);
    VehicleComponent(size_t _wheelCount, bool _inputTypeDigital);
    VehicleComponent();

    ComponentType GetType();
    void HandleEvent(Event *event);

    bool inAir;
    physx::PxVehicleDrive4W* pxVehicle = nullptr;
    physx::PxVehicleDrive4WRawInputData pxVehicleInputData;
    bool inputTypeDigital;

    void SetEntity(Entity* _entity) override;

    void UpdateFromPhysics(physx::PxTransform t) override;
    void UpdateWheelTransforms();

    float GetChassisMass() const;
    glm::vec3 GetChassisSize() const;
    glm::vec3 GetChassisMomentOfInertia() const;
    glm::vec3 GetChassisCenterOfMassOffset() const;

    float GetWheelMass() const;
    float GetWheelRadius() const;
    float GetWheelWidth() const;
    float GetWheelMomentOfIntertia() const;
    size_t GetWheelCount() const;

    std::vector<AxleData> GetAxleData() const;

    void RenderDebugGui() override;

private:
    MeshComponent* wheelMeshPrefab;
    std::vector<MeshComponent*> wheelMeshes;
    std::vector<Collider*> wheelColliders;

    physx::PxVehicleDriveSimData4W driveSimData;
    physx::PxVehicleWheelsSimData* wheelsSimData;

    glm::vec3 chassisSize;

    float wheelMass;
    float wheelRadius;
    float wheelWidth;
    size_t wheelCount;

    std::vector<AxleData> axleData;

    void Initialize();
    void CreateVehicle();
    void InitializeWheelsSimulationData(const physx::PxVec3* wheelCenterActorOffsets);
};
