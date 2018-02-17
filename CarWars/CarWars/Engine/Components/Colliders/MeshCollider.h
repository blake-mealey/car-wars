#pragma once

#include "Collider.h"
#include <json/json.hpp>

class Mesh;

class MeshCollider : public Collider {
public:
    MeshCollider(std::string _collisionGroup, physx::PxMaterial *_material, Mesh *_mesh);
    MeshCollider(nlohmann::json data);

    ColliderType GetType() const override;
private:
    void InitializeGeometry() override;
    Mesh *mesh;
};
