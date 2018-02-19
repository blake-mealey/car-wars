#pragma once

#include "Collider.h"
#include <glm/glm.hpp>
#include <json/json.hpp>

class Mesh;

class ConvexMeshCollider : public Collider {
public:
    ConvexMeshCollider(std::string _collisionGroup, physx::PxMaterial *_material, physx::PxFilterData _queryFilterData, Mesh *_mesh);
    ConvexMeshCollider(std::string _collisionGroup, physx::PxMaterial *_material, physx::PxFilterData _queryFilterData, physx::PxConvexMesh *_mesh);
    ConvexMeshCollider(nlohmann::json data);

    ColliderType GetType() const override;

    Mesh* GetRenderMesh() override;
private:
    void InitializeGeometry(Mesh *mesh);
    void InitializeGeometry(physx::PxConvexMesh *mesh);
};