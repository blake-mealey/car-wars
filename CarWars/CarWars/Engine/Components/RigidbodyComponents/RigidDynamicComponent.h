#pragma once

#include "RigidbodyComponent.h"
#include <PxRigidDynamic.h>

class RigidDynamicComponent : public RigidbodyComponent<RigidDynamicComponent> {
	friend class RigidbodyComponent<RigidDynamicComponent>;
public:
    RigidDynamicComponent(float _mass=100.f, glm::vec3 _momentOfIntertia=glm::vec3(0.f), glm::vec3 _centerOfMassOffset=glm::vec3(0.f));
	RigidDynamicComponent();
	RigidDynamicComponent(nlohmann::json data);

    void SetMass(float _mass);
    void SetMomentOfInertia(glm::vec3 _momentOfInertia);
    void SetCenterOfMassOffset(glm::vec3 _centerOfMassOffset);

    static constexpr ComponentType InternalGetType() { return ComponentType_RigidDynamic; }
    //void HandleEvent(Event *event) override;

    void InternalRenderDebugGui();

    physx::PxRigidDynamic *actor;

protected:
    void InternalInitializeRigidbody();

    float mass;
    glm::vec3 momentOfInertia;
    glm::vec3 centerOfMassOffset;
};
