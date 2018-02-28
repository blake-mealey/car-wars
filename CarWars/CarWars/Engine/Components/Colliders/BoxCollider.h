#pragma once

#include "Collider.h"
#include <glm/glm.hpp>
#include <json/json.hpp>

class BoxCollider : public Collider {
public:
    BoxCollider(std::string _collisionGroup, physx::PxMaterial *_material, physx::PxFilterData _queryFilterData, glm::vec3 _scale);
    BoxCollider(nlohmann::json data);

    ColliderType GetType() const override;

    Mesh* GetRenderMesh() override;

    Transform GetLocalTransform() const override;
    Transform GetGlobalTransform() const override;
private:
    void InitializeGeometry();
};
