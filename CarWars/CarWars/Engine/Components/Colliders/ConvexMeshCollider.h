#pragma once

#include "Collider.h"
#include <glm/glm.hpp>
#include <json/json.hpp>

class Mesh;

class ConvexMeshCollider : public Collider {
public:
    ConvexMeshCollider(std::string _collisionGroup, physx::PxMaterial *_material, Mesh *_mesh);
    ConvexMeshCollider(nlohmann::json data);

private:
    void InitializeGeometry() override;
    Mesh *mesh;
};
