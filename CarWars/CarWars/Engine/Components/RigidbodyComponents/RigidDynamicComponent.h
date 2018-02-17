#pragma once

#include "RigidbodyComponent.h"
#include <PxRigidDynamic.h>

class RigidDynamicComponent : public RigidbodyComponent {
public:
    RigidDynamicComponent(float _mass, glm::vec3 _momentOfIntertia, glm::vec3 _centerOfMassOffset);
    RigidDynamicComponent(nlohmann::json data);

    void SetMass(float _mass);

    ComponentType GetType() override;
    void HandleEvent(Event *event) override;

    void RenderDebugGui() override;

private:
    void InitializeRigidbody() override;

    physx::PxRigidDynamic *actor;
    float mass;
    glm::vec3 momentOfInertia;
    glm::vec3 centerOfMassOffset;
};
