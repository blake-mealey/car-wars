#include "RigidDynamicComponent.h"
#include "../../Systems/Physics.h"
#include "../../Entities/Transform.h"
#include "../../Systems/Content/ContentManager.h"
#include "imgui/imgui.h"
#include <glm/gtc/type_ptr.inl>

RigidDynamicComponent::RigidDynamicComponent(float _mass, glm::vec3 _momentOfIntertia, glm::vec3 _centerOfMassOffset) :
    mass(_mass), momentOfInertia(_momentOfIntertia), centerOfMassOffset(_centerOfMassOffset) {
    
    RigidDynamicComponent::InitializeRigidbody();
}

RigidDynamicComponent::RigidDynamicComponent(nlohmann::json data) : RigidbodyComponent(data) {
    mass = ContentManager::GetFromJson<float>(data["Mass"], 100.f);
    momentOfInertia = ContentManager::JsonToVec3(data["MomentOfIntertia"], glm::vec3(20.f));
    centerOfMassOffset = ContentManager::JsonToVec3(data["CenterOfMassOffset"], glm::vec3(0.f));
    
    RigidDynamicComponent::InitializeRigidbody();
}

RigidDynamicComponent::~RigidDynamicComponent() {
	pxRigid->release();
}

void RigidDynamicComponent::SetMass(float _mass) {
    mass = _mass;
    actor->setMass(mass);
}

void RigidDynamicComponent::SetMomentOfInertia(glm::vec3 _momentOfInertia) {
    momentOfInertia = _momentOfInertia;
    actor->setMassSpaceInertiaTensor(Transform::ToPx(momentOfInertia));
}

void RigidDynamicComponent::SetCenterOfMassOffset(glm::vec3 _centerOfMassOffset) {
    centerOfMassOffset = _centerOfMassOffset;
    actor->setCMassLocalPose(PxTransform(Transform::ToPx(centerOfMassOffset), PxQuat(PxIdentity)));
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
    if (ImGui::DragFloat("Mass", &mass, 10, 0)) SetMass(mass);
    if (ImGui::DragFloat3("MomentOfInertia", glm::value_ptr(momentOfInertia), 0.01)) SetMomentOfInertia(momentOfInertia);
    if (ImGui::DragFloat3("CenterOfMassOffset", glm::value_ptr(centerOfMassOffset), 0.01)) SetCenterOfMassOffset(centerOfMassOffset);

	glm::vec3 linearVelocity = Transform::FromPx(actor->getLinearVelocity());
	if (ImGui::DragFloat3("Linear Velocity", glm::value_ptr(linearVelocity), 0.1f)) actor->setLinearVelocity(Transform::ToPx(linearVelocity));

	glm::vec3 angularVelocity = Transform::FromPx(actor->getAngularVelocity());
	if (ImGui::DragFloat3("Angular Velocity", glm::value_ptr(angularVelocity), 0.1f)) actor->setAngularVelocity(Transform::ToPx(angularVelocity));

}
