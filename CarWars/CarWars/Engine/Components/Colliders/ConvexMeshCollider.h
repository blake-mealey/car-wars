#pragma once

#include "Collider.h"
#include <glm/glm.hpp>
#include <json/json.hpp>

class Mesh;

class ConvexMeshCollider : public Collider {
public:
    ~ConvexMeshCollider() override;
    ConvexMeshCollider(std::string _collisionGroup, physx::PxMaterial *_material, physx::PxFilterData _queryFilterData, Mesh *_mesh);
    ConvexMeshCollider(std::string _collisionGroup, physx::PxMaterial *_material, physx::PxFilterData _queryFilterData, physx::PxConvexMesh *_mesh);
    ConvexMeshCollider(nlohmann::json data);

    ColliderType GetType() const override;

    Mesh* GetRenderMesh() override;
protected:
	void InitializeGeometry() override;
private:
	physx::PxConvexMesh* convexMesh;
    Mesh *renderMesh;

    void InitializeGeometry(Mesh *mesh);

    void InitializeRenderMesh();
};