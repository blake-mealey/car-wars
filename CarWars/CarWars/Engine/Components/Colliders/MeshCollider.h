#pragma once

#include "Collider.h"
#include <json/json.hpp>

class Mesh;

class MeshCollider : public Collider {
public:
    MeshCollider(std::string _collisionGroup, physx::PxMaterial *_material, physx::PxFilterData _queryFilterData, Mesh *_mesh);
    MeshCollider(nlohmann::json data);

    ColliderType GetType() const override;
private:
    void InitializeGeometry();
    Mesh *mesh;
};