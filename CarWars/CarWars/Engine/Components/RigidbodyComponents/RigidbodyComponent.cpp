#include "RigidbodyComponent.h"
#include "../../Systems/Game.h"
#include "../../Systems/Physics.h"
#include "../../Entities/Transform.h"
#include "../../Systems/Content/ContentManager.h"
#include "../Colliders/BoxCollider.h"
#include "../Colliders/ConvexMeshCollider.h"
#include "../Colliders/MeshCollider.h"
#include "../Colliders/SphereCollider.h"
#include "imgui/imgui.h"

RigidbodyComponent::~RigidbodyComponent() {
	NavigationMesh *navMesh = Game::Instance().GetNavigationMesh();
	if (navMesh) navMesh->RemoveRigidbody(this);
    Physics::Instance().GetScene().removeActor(static_cast<physx::PxActor&>(*pxRigid));
    for (Collider *collider : colliders) {
        delete collider;
    }
    pxRigid->release();
}

RigidbodyComponent::RigidbodyComponent() : pxRigid(nullptr), blocksNavigationMesh(true) {}

RigidbodyComponent::RigidbodyComponent(nlohmann::json data, Mesh* heightmap) : RigidbodyComponent() {
	blocksNavigationMesh = ContentManager::GetFromJson<bool>(data["BlocksNavMesh"], true);
	if (heightmap == nullptr) {
		for (nlohmann::json colliderData : data["Colliders"]) {
			std::string type = colliderData["Type"];
			Collider *collider = nullptr;

			if (type == "Box") collider = new BoxCollider(colliderData);
			else if (type == "ConvexMesh") collider = new ConvexMeshCollider(colliderData);
			else if (type == "Mesh") collider = new MeshCollider(colliderData);
			else if (type == "Sphere") collider = new SphereCollider(colliderData);

			if (collider) AddCollider(collider);
		}
	}
	else {
		for (nlohmann::json colliderData : data["Colliders"]) {
			//std::string type = colliderData["Type"];
			Collider *collider = nullptr;
			collider = new MeshCollider(colliderData, heightmap);
			if (collider) AddCollider(collider);
		}
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
}

bool RigidbodyComponent::DoesBlockNavigationMesh() const {
    return blocksNavigationMesh;
}

void RigidbodyComponent::OnContact(RigidbodyComponent* body) { }
void RigidbodyComponent::OnTrigger(RigidbodyComponent* body) { }
