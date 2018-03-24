#pragma once

#include "Collider.h"
#include <json/json.hpp>

class Mesh;

class MeshCollider : public Collider {
public:
    ~MeshCollider() override;
    MeshCollider(std::string _collisionGroup, physx::PxMaterial *_material, physx::PxFilterData _queryFilterData, bool _isTrigger, Mesh *_mesh);
    MeshCollider(nlohmann::json data);

    ColliderType GetType() const override;

    Mesh* GetRenderMesh() override;

protected:
	void InitializeGeometry() override;

private:
	physx::PxTriangleMesh *triangleMesh;
	Mesh *mesh;
	
    void InitializeGeometry(Mesh *renderMesh);
};