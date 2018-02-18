#include "ConvexMeshCollider.h"
#include <cooking/PxConvexMeshDesc.h>
#include "../../Systems/Content/Mesh.h"
#include "../../Systems/Physics.h"
#include "../../Systems/Content/ContentManager.h"

using namespace physx;

ConvexMeshCollider::ConvexMeshCollider(std::string _collisionGroup, physx::PxMaterial *_material, physx::PxFilterData _queryFilterData, Mesh *_mesh)
    : Collider(_collisionGroup, _material, _queryFilterData) {
    
    InitializeGeometry(_mesh);
}

ConvexMeshCollider::ConvexMeshCollider(std::string _collisionGroup, physx::PxMaterial* _material, physx::PxFilterData _queryFilterData, physx::PxConvexMesh* _mesh)
    : Collider(_collisionGroup, _material, _queryFilterData) {
    
    InitializeGeometry(_mesh);
}

ConvexMeshCollider::ConvexMeshCollider(nlohmann::json data) : Collider(data) {
    InitializeGeometry(ContentManager::GetMesh(data["Mesh"]));
}

ColliderType ConvexMeshCollider::GetType() const {
    return Collider_ConvexMesh;
}

void ConvexMeshCollider::InitializeGeometry(Mesh *mesh) {
    PxConvexMeshDesc convexDesc;
    convexDesc.points.count = mesh->vertexCount;
    convexDesc.points.stride = sizeof(glm::vec3);
    convexDesc.points.data = mesh->vertices;
    convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

    PxConvexMesh* convexMesh = nullptr;
    PxDefaultMemoryOutputStream buf;
    Physics& physics = Physics::Instance();
    if (physics.GetCooking().cookConvexMesh(convexDesc, buf)) {
        PxDefaultMemoryInputData id(buf.getData(), buf.getSize());
        convexMesh = physics.GetApi().createConvexMesh(id);
    }

    geometry = new PxConvexMeshGeometry(convexMesh);
}

void ConvexMeshCollider::InitializeGeometry(physx::PxConvexMesh* mesh) {
    geometry = new PxConvexMeshGeometry(mesh);
}