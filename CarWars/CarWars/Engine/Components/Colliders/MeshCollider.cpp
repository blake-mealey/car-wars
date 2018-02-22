#include "MeshCollider.h"
#include <cooking/PxConvexMeshDesc.h>
#include "../../Systems/Content/Mesh.h"
#include "../../Systems/Physics.h"
#include "../../Systems/Content/ContentManager.h"
#include <iostream>

using namespace physx;

MeshCollider::MeshCollider(std::string _collisionGroup, physx::PxMaterial *_material, physx::PxFilterData _queryFilterData, Mesh *_mesh)
    : Collider(_collisionGroup, _material, _queryFilterData), mesh(_mesh) {
    
    InitializeGeometry();
}

MeshCollider::MeshCollider(nlohmann::json data) : Collider(data) {
    mesh = ContentManager::GetMesh(data["Mesh"])->TransformMesh(Transform(nullptr, glm::vec3(0.f), transform.GetLocalScale(), glm::vec3(0.f), false));
    InitializeGeometry();
}

ColliderType MeshCollider::GetType() const {
    return Collider_TriangleMesh;
}

Mesh* MeshCollider::GetRenderMesh() {
    return mesh;
}

void MeshCollider::InitializeGeometry() {
    PxTriangleMeshDesc meshDesc;
    meshDesc.flags |= PxMeshFlag::e16_BIT_INDICES;

    meshDesc.points.count = mesh->vertexCount;
    meshDesc.points.stride = sizeof(glm::vec3);
    meshDesc.points.data = mesh->vertices;

    meshDesc.triangles.count = mesh->triangleCount;
    meshDesc.triangles.stride = sizeof(Triangle);
    meshDesc.triangles.data = mesh->triangles;

    Physics& physics = Physics::Instance();

    PxDefaultMemoryOutputStream writeBuffer;
    PxTriangleMeshCookingResult::Enum result;
    const bool status = physics.GetCooking().cookTriangleMesh(meshDesc, writeBuffer, &result);
    if (!status)
        std::cout << "Failed to initialize mesh geometry";

    PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
    geometry = new PxTriangleMeshGeometry(physics.GetApi().createTriangleMesh(readBuffer));
}
