#include "RigidStaticComponent.h"
#include "../../Systems/Physics.h"

RigidStaticComponent::RigidStaticComponent(nlohmann::json data) : RigidbodyComponent(data) {
    RigidStaticComponent::InitializeRigidbody();
}

void RigidStaticComponent::InitializeRigidbody() {
    // Create the rigidbody
    Physics &physics = Physics::Instance();
    PxRigidStatic *actor = physics.GetApi().createRigidStatic(PxTransform(PxIdentity));

    RigidbodyComponent::InitializeRigidbody(actor);
}

ComponentType RigidStaticComponent::GetType() {
    return ComponentType_RigidStatic;
}

void RigidStaticComponent::HandleEvent(Event* event) { }

void RigidStaticComponent::RenderDebugGui() {
    RigidbodyComponent::RenderDebugGui();
}
