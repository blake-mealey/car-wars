#pragma once

#include "../Component.h"
#include "../Colliders/Collider.h"

#include "glm/glm.hpp"
#include <json/json.hpp>

class RigidbodyComponent : public Component {
public:
    RigidbodyComponent();
    RigidbodyComponent(nlohmann::json data);

    void AddCollider(Collider *collider);

	ComponentType GetType() override;
	void HandleEvent(Event *event) override;

    void RenderDebugGui() override;
    void SetEntity(Entity *_entity) override;
protected:
    virtual void InitializeRigidbody() = 0;
    void InitializeRigidbody(physx::PxRigidActor* actor);

    std::vector<Collider*> colliders;
    physx::PxRigidActor* pxRigid;
};
