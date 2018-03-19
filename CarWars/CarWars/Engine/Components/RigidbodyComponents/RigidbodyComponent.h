#pragma once

#include "../Component.h"
#include "../Colliders/Collider.h"

#include "glm/glm.hpp"
#include <json/json.hpp>

class RigidbodyComponent : public Component {
public:
    ~RigidbodyComponent() override;
    RigidbodyComponent();
    RigidbodyComponent(nlohmann::json data);
	RigidbodyComponent(Mesh* mesh, nlohmann::json data);

    void AddCollider(Collider *collider);

	ComponentType GetType() override;
	void HandleEvent(Event *event) override;

    void RenderDebugGui() override;
    void SetEntity(Entity *_entity) override;

    bool DoesBlockNavigationMesh() const;

    std::vector<Collider*> colliders;
    physx::PxRigidActor* pxRigid;
protected:
    virtual void InitializeRigidbody() = 0;
    void InitializeRigidbody(physx::PxRigidActor* actor);

    bool blocksNavigationMesh;

};
