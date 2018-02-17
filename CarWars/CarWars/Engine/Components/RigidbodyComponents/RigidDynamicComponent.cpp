#include "RigidDynamicComponent.h"
#include "../../Systems/Physics.h"
#include "../../Entities/Transform.h"
#include "../../Systems/Content/ContentManager.h"
#include "imgui/imgui.h"

RigidDynamicComponent::RigidDynamicComponent(float _mass, glm::vec3 _momentOfIntertia, glm::vec3 _centerOfMassOffset) :
    mass(_mass), momentOfInertia(_momentOfIntertia), centerOfMassOffset(_centerOfMassOffset) {
    
    RigidDynamicComponent::InitializeRigidbody();
}

RigidDynamicComponent::RigidDynamicComponent(nlohmann::json data) : RigidbodyComponent(data) {
    mass = ContentManager::GetFromJson<float>(data["Mass"], 100.f);
    momentOfInertia = ContentManager::JsonToVec3(data["MomentOfIntertia"], glm::vec3(0.f));
    centerOfMassOffset = ContentManager::JsonToVec3(data["CenterOfMassOffset"], glm::vec3(0.f));
    
    RigidDynamicComponent::InitializeRigidbody();
}

void RigidDynamicComponent::SetMass(float _mass) {
    mass = _mass;
    actor->setMass(mass);
}

void RigidDynamicComponent::InitializeRigidbody() {
    // Create the rigidbody
    Physics &physics = Physics::Instance();
    actor = physics.GetApi().createRigidDynamic(PxTransform(PxIdentity));

    // Setup the rigidbody
    actor->setMass(mass);
    actor->setMassSpaceInertiaTensor(Transform::ToPx(momentOfInertia));
    actor->setCMassLocalPose(PxTransform(Transform::ToPx(centerOfMassOffset), PxQuat(PxIdentity)));

    RigidbodyComponent::InitializeRigidbody(actor);
}

ComponentType RigidDynamicComponent::GetType() {
    return ComponentType_RigidDynamic;
}

void RigidDynamicComponent::HandleEvent(Event* event) {}

void RigidDynamicComponent::RenderDebugGui() {
    RigidbodyComponent::RenderDebugGui();
    if (ImGui::DragFloat("Mass", &mass, 10, 0)) actor->setMass(mass);
}
