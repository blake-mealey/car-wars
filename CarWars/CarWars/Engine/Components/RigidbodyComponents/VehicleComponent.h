#pragma once

#include <vector>

#include <vehicle/PxVehicleDrive4W.h>
#include <vehicle/PxVehicleUtilControl.h>
#include <json/json.hpp>
#include "RigidDynamicComponent.h"
#include "../MeshComponent.h"

class WeaponComponent;

struct AxleData {
    AxleData(const float _centerOffset = 0.f, const float _wheelInset = 0.f)
        : centerOffset(_centerOffset), wheelInset(_wheelInset) {};
    float centerOffset;
    float wheelInset;
};

class VehicleComponent : public RigidDynamicComponent {
public:
    ~VehicleComponent() override;
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

    float speedMultiplier = 1.f;
    float defenceMultiplier = 1.f;
	void TakeDamage(WeaponComponent* damager, float damage) override;
	float GetHealth();

	size_t GetRaycastGroup() const;

	void Boost(glm::vec3 boostDir);
	void HandleAcceleration( float forwardPower, float backwardPower);
	void Steer( float amount);
	void Handbrake( float amount);

    void PoweredUp();

	Time GetTimeSinceBoost();

private:
    Time powerUpLife;
	Time lastBoost;
	Time boostCooldown;

	float boostPower;

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

	float health = 1000.f;
	float resistance = 0.5f;

	size_t raycastGroup;

    void Initialize();
    void CreateVehicle();
    void InitializeWheelsSimulationData(const physx::PxVec3* wheelCenterActorOffsets);
};
