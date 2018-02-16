#pragma once

#include <vector>

#include "Component.h"
#include "MeshComponent.h"
#include <vehicle/PxVehicleDrive4W.h>
#include <vehicle/PxVehicleUtilControl.h>
#include <json/json.hpp>

class VehicleComponent : public Component {
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

	float GetFrontAxisOffset() const;
	float GetRearAxisOffset() const;

private:
	MeshComponent* wheelMeshPrefab;
    std::vector<MeshComponent*> wheelMeshes;

    float chassisMass;
    glm::vec3 chassisSize;

    float wheelMass;
    float wheelRadius;
    float wheelWidth;
	size_t wheelCount;

	float frontAxisOffset;
	float rearAxisOffset;

	void Initialize();
};
