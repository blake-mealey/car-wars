#pragma once
#include "../Component.h"
#include "../../Systems/Physics.h"
#include "../Colliders/Collider.h"
#include "../Colliders/BoxCollider.h"
#include "../Colliders/ConvexMeshCollider.h"
#include "../Colliders/MeshCollider.h"

#include "glm/glm.hpp"
#include <json/json.hpp>

template<class T, EventType... events>
class RigidbodyComponent : public Component<T, events...> {
	friend class Component<T, events>;
public:
	std::vector<Collider*> colliders;

	RigidbodyComponent() : pxRigid(nullptr) {};
	RigidbodyComponent(nlohmann::json data) : RigidbodyComponent() {
		for (nlohmann::json colliderData : data["Colliders"]) {
			std::string type = colliderData["Type"];
			Collider *collider = nullptr;

			if (type == "Box") collider = new BoxCollider(colliderData);
			else if (type == "ConvexMesh") collider = new ConvexMeshCollider(colliderData);
			else if (type == "Mesh") collider = new MeshCollider(colliderData);

			if (collider) AddCollider(collider);
		}
	}

	void AddCollider(Collider* collider) {
		colliders.push_back(collider);
		if (pxRigid) collider->CreateShape(pxRigid);
	}
	//void HandleEvent(Event *event) override;

    void InternalRenderDebugGui() {
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
    void InternalSetEntity(Entity& _entity) {
		pxRigid->setGlobalPose(Transform::ToPx(EntityManager::transforms[_entity.transformID]));
		Physics::Instance().GetScene().addActor(*pxRigid);
	}
protected:
	physx::PxRigidActor* pxRigid;

	void InitializeRigidbody() { static_cast<T*>(this)->InternalInitializeRigidbody(); }
    void InitializeRigidbody(physx::PxRigidActor* actor) {
		pxRigid = actor;
		pxRigid->userData = this;

		for (Collider *collider : colliders) {
			collider->CreateShape(pxRigid);
		}
	}
};
