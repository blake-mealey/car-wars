#include "RigidbodyComponent.h"
#include "../../Systems/Physics.h"
#include "../../Entities/Transform.h"
#include "../../Systems/Content/ContentManager.h"
#include "../Colliders/BoxCollider.h"
#include "../Colliders/ConvexMeshCollider.h"
#include "../Colliders/MeshCollider.h"
#include "imgui/imgui.h"
#include "../../Entities/EntityManager.h"
#include "RigidDynamicComponent.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>
#define _USE_MATH_DEFINES
#include <math.h>

RigidbodyComponent::RigidbodyComponent() : pxRigid(nullptr), blocksNavigationMesh(true) {}

RigidbodyComponent::RigidbodyComponent(nlohmann::json data) : RigidbodyComponent() {
    blocksNavigationMesh = ContentManager::GetFromJson<bool>(data["BlocksNavMesh"], true);

    for (nlohmann::json colliderData : data["Colliders"]) {
        std::string type = colliderData["Type"];
        Collider *collider = nullptr;
        
        if (type == "Box") collider = new BoxCollider(colliderData);
        else if (type == "ConvexMesh") collider = new ConvexMeshCollider(colliderData);
        else if (type == "Mesh") collider = new MeshCollider(colliderData);
        
        if (collider) AddCollider(collider);
    }
}

void RigidbodyComponent::AddCollider(Collider* collider) {
    colliders.push_back(collider);
	if (pxRigid) collider->CreateShape(pxRigid);
}

void RigidbodyComponent::InitializeRigidbody(physx::PxRigidActor* actor) {
    pxRigid = actor;
    pxRigid->userData = this;
    
    for (Collider *collider : colliders) {
        collider->CreateShape(pxRigid);
    }
}

ComponentType RigidbodyComponent::GetType() {
	return ComponentType_Rigidbody;
}

void RigidbodyComponent::HandleEvent(Event *event) {}

void RigidbodyComponent::RenderDebugGui() {
    Component::RenderDebugGui();
    if (ImGui::TreeNode("Transform")) {
        Transform t = pxRigid->getGlobalPose();
        if (t.RenderDebugGui()) pxRigid->setGlobalPose(Transform::ToPx(t), true);
        ImGui::TreePop();
    }

    if (!colliders.empty() && ImGui::TreeNode("Colliders")) {
        size_t i = 0;
        for (Collider *collider : colliders) {
            if (ImGui::TreeNode((void*)(intptr_t)i, "Collider (%s)", Collider::GetTypeName(collider->GetType()))) {
                collider->RenderDebugGui();
                ImGui::TreePop();
            }
            i++;
        }
        ImGui::TreePop();
    }
}

void RigidbodyComponent::SetEntity(Entity* _entity) {
    Component::SetEntity(_entity);
    pxRigid->setGlobalPose(Transform::ToPx(_entity->transform));
    Physics::Instance().GetScene().addActor(*pxRigid);

	for (Collider *collider : colliders) {
		collider->Scale(_entity->transform.GetLocalScale());
	}
	if (_entity->connectedToCylinder) {
		Entity* cylinder = EntityManager::FindEntities("Cylinder")[0];
		Physics &physics = Physics::Instance();

		RigidDynamicComponent *entityRigid = static_cast<RigidDynamicComponent*>(_entity->components[1]);

		// Don't let forces move the object
		entityRigid->actor->setAngularDamping(0.f);
		entityRigid->actor->setMassSpaceInertiaTensor(PxVec3(0.f, 0.f, 0.f));

		auto pos = _entity->transform.GetGlobalPosition();
		pos.x = pos.x;
		pos = (Transform::ToCylinder(pos));
		//rotate accordingly
		float rotBy = _entity->transform.GetGlobalPosition().x / (Transform::radius *2.f*(float)M_PI);
		auto cylinderJointRotation = Transform::ToPx(glm::rotate(glm::quat(), rotBy + (float)M_PI, glm::vec3(0, 0, 1)));
		auto entityJointRotation = glm::rotate(glm::quat(), rotBy, glm::vec3(0, 0, 1));

		_entity->transform = Transform(nullptr, pos, _entity->transform.GetGlobalScale(), entityJointRotation);

		PxFixedJoint* joint = PxFixedJointCreate(physics.GetApi(),
			static_cast<RigidDynamicComponent*>(cylinder->components[1])->actor, PxTransform(PxVec3(0), cylinderJointRotation),
			pxRigid, Transform::ToPx(_entity->transform));

		entityRigid->actor->setAngularVelocity(PxVec3(0.f, 0.f, 0.06f));

		// compute new transform
		// x -> 0
		// y -> add radius
		// z -> same

		// rotation based on x
	}
}

bool RigidbodyComponent::DoesBlockNavigationMesh() const {
    return blocksNavigationMesh;
}
