#include "ConvexMeshCollider.h"
#include <cooking/PxConvexMeshDesc.h>
#include "../../Systems/Content/Mesh.h"
#include "../../Systems/Physics.h"
#include "../../Systems/Content/ContentManager.h"

using namespace physx;

ConvexMeshCollider::ConvexMeshCollider(std::string _collisionGroup, physx::PxMaterial *_material, Mesh *_mesh) : Collider(_collisionGroup, _material), mesh(_mesh) {
    ConvexMeshCollider::InitializeGeometry();
}

ConvexMeshCollider::ConvexMeshCollider(nlohmann::json data) : Collider(data) {
    mesh = ContentManager::GetMesh(data["Mesh"]);
    ConvexMeshCollider::InitializeGeometry();
}

ColliderType ConvexMeshCollider::GetType() const {
    return Collider_ConvexMesh;
}

void ConvexMeshCollider::InitializeGeometry() {
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

