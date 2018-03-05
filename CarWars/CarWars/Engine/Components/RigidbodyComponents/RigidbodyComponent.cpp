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
#include "RigidStaticComponent.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>
#define _USE_MATH_DEFINES
#include <math.h>

#include <iostream>
using namespace std;

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
	Transform position = _entity->transform;
	
	Component::SetEntity(_entity);

    pxRigid->setGlobalPose(Transform::ToPx(position));
	
	if (_entity->connectedToCylinder) {
		Entity* cylinder = EntityManager::FindEntities("Cylinder")[0];
		Physics &physics = Physics::Instance();

		const RigidDynamicComponent *entityRigid = static_cast<RigidDynamicComponent*>(_entity->components[1]);
		entityRigid->colliders[0]->GetShape()->setLocalPose(Transform::ToPx(_entity->transform));
		auto * shape = entityRigid->colliders[0]->GetShape();
		static_cast<RigidDynamicComponent*>(cylinder->components[1])->pxRigid->attachShape(*shape);
	}
	else {
		Physics::Instance().GetScene().addActor(*pxRigid);
	}

	for (Collider *collider : colliders) {
		collider->Scale(_entity->transform.GetLocalScale());
	}
}

bool RigidbodyComponent::DoesBlockNavigationMesh() const {
    return blocksNavigationMesh;
}
