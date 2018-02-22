#include "ConvexMeshCollider.h"
#include <cooking/PxConvexMeshDesc.h>
#include "../../Systems/Content/Mesh.h"
#include "../../Systems/Physics.h"
#include "../../Systems/Content/ContentManager.h"

using namespace physx;

ConvexMeshCollider::ConvexMeshCollider(std::string _collisionGroup, physx::PxMaterial *_material, physx::PxFilterData _queryFilterData, Mesh *_mesh)
    : Collider(_collisionGroup, _material, _queryFilterData) {
    
    _mesh = _mesh->TransformMesh(Transform(nullptr, glm::vec3(0.f), transform.GetLocalScale(), glm::vec3(0.f), false));
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

Mesh* ConvexMeshCollider::GetRenderMesh() {
    return renderMesh;
}

void ConvexMeshCollider::InitializeGeometry(Mesh *mesh) {
    PxConvexMeshDesc convexDesc;
    convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX | PxConvexFlag::e16_BIT_INDICES;

    convexDesc.points.count = mesh->vertexCount;
    convexDesc.points.stride = sizeof(glm::vec3);
    convexDesc.points.data = mesh->vertices;

    /*convexDesc.indices
    convexDesc.triangles.count = mesh->triangleCount;
    convexDesc.triangles.stride = sizeof(Triangle);
    convexDesc.triangles.data = mesh->triangles;
    convexDesc.indices.count = mesh->triangleCount;
    convexDesc.indices.stride = mesh->triangleCount;*/

    PxConvexMesh* convexMesh = nullptr;
    PxDefaultMemoryOutputStream buf;
    Physics& physics = Physics::Instance();
    if (physics.GetCooking().cookConvexMesh(convexDesc, buf)) {
        PxDefaultMemoryInputData id(buf.getData(), buf.getSize());
        convexMesh = physics.GetApi().createConvexMesh(id);
    }

    InitializeGeometry(convexMesh);
}

void ConvexMeshCollider::InitializeGeometry(PxConvexMesh* mesh) {
    InitializeRenderMesh(mesh);
    geometry = new PxConvexMeshGeometry(mesh);
}

void ConvexMeshCollider::InitializeRenderMesh(PxConvexMesh* mesh) {
    /*const PxU32 triangleCount = mesh->getNbPolygons();
    const PxU8 *indexBuffer = mesh->getIndexBuffer();
    Triangle *triangles = new Triangle[triangleCount];
    for (size_t i = 0; i < triangleCount; ++i) {
        PxHullPolygon poly;
        mesh->getPolygonData(i, poly);
        assert(poly.mNbVerts == 3);
        const PxU16 indexBase = poly.mIndexBase;
        triangles[i] = Triangle(indexBuffer[indexBase + 0],
                                indexBuffer[indexBase + 1],
                                indexBuffer[indexBase + 2]);
    }*/


    /*const PxU32 triangleCount = mesh->getNbPolygons();
    const PxU8 *indexBuffer = mesh->getIndexBuffer();
    Triangle *triangles = new Triangle[triangleCount];
    for (size_t i = 0; i < triangleCount * 3; ++i) {
        triangles[i] = Triangle(indexBuffer[i * 3 + 0],
            indexBuffer[i * 3 + 1],
            indexBuffer[i * 3 + 2]);
    }

    const PxU32 vertexCount = mesh->getNbVertices();
    const PxVec3 *verts = mesh->getVertices();
    glm::vec3 *vertices = new glm::vec3[vertexCount];
    for (size_t i = 0; i < vertexCount; ++i) {
        vertices[i] = Transform::FromPx(verts[i]);
    }

    renderMesh = new Mesh(triangleCount, vertexCount, triangles, vertices);*/
    renderMesh = nullptr;
}
