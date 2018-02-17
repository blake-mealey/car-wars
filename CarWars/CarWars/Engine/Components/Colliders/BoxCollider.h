#pragma once

#include "Collider.h"
#include <glm/glm.hpp>
#include <json/json.hpp>

class BoxCollider : public Collider {
public:
    BoxCollider(std::string _collisionGroup, physx::PxMaterial *_material, glm::vec3 _size);
    BoxCollider(nlohmann::json data);

    ColliderType GetType() const override;

    Transform GetGlobalTransform() const override;
private:
    void InitializeGeometry() override;
    glm::vec3 size;
};
