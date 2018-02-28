#include "RigidStaticComponent.h"
#include "../../Systems/Physics.h"

RigidStaticComponent::RigidStaticComponent() : RigidbodyComponent() {}

RigidStaticComponent::RigidStaticComponent(nlohmann::json data) : RigidbodyComponent(data) {
    RigidStaticComponent::InitializeRigidbody();
}

void RigidStaticComponent::InternalInitializeRigidbody() {
    // Create the rigidbody
    Physics &physics = Physics::Instance();
    PxRigidStatic *actor = physics.GetApi().createRigidStatic(PxTransform(PxIdentity));

    RigidbodyComponent::InitializeRigidbody(actor);
}

void RigidStaticComponent::InternalRenderDebugGui() {
    RigidbodyComponent::RenderDebugGui();
}
