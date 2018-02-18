#include "MeshCollider.h"
#include <cooking/PxConvexMeshDesc.h>
#include "../../Systems/Content/Mesh.h"
#include "../../Systems/Physics.h"
#include "../../Systems/Content/ContentManager.h"

using namespace physx;

MeshCollider::MeshCollider(std::string _collisionGroup, physx::PxMaterial *_material, physx::PxFilterData _queryFilterData, Mesh *_mesh)
    : Collider(_collisionGroup, _material, _queryFilterData), mesh(_mesh) {
    
    InitializeGeometry();
}

MeshCollider::MeshCollider(nlohmann::json data) : Collider(data) {
    mesh = ContentManager::GetMesh(data["Mesh"]);
    InitializeGeometry();
}

ColliderType MeshCollider::GetType() const {
    return Collider_TriangleMesh;
}

void MeshCollider::InitializeGeometry() {
    PxTriangleMeshDesc meshDesc;
    meshDesc.points.count = mesh->vertexCount;
    meshDesc.points.stride = sizeof(glm::vec3);
    meshDesc.points.data = mesh->vertices;

    // Needed?
    /*meshDesc.triangles.count = mesh->triangleCount;
    meshDesc.triangles.stride = 3 * sizeof(glm::vec3);
    meshDesc.triangles.data = mesh->triangles;*/

    PxTriangleMesh* triangleMesh = nullptr;
    PxDefaultMemoryOutputStream buf;
    Physics& physics = Physics::Instance();
    if (physics.GetCooking().cookTriangleMesh(meshDesc, buf)) {
        PxDefaultMemoryInputData id(buf.getData(), buf.getSize());
        triangleMesh = physics.GetApi().createTriangleMesh(id);
    }

    geometry = new PxTriangleMeshGeometry(triangleMesh);
}
