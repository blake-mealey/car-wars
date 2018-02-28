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

Mesh* ConvexMeshCollider::GetRenderMesh() {
    return renderMesh;
}

void ConvexMeshCollider::InitializeGeometry(Mesh *mesh) {
    PxConvexMeshDesc convexDesc;
    convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX | PxConvexFlag::e16_BIT_INDICES;

	glm::vec3 *vertices = new glm::vec3[mesh->vertexCount];
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbos[VBOs::Vertices]);
	glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * mesh->vertexCount, vertices);

    convexDesc.points.count = mesh->vertexCount;
    convexDesc.points.stride = sizeof(glm::vec3);
    convexDesc.points.data = vertices;

	glm::vec3 *triangles = new glm::vec3[mesh->triangleCount];
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->eabs[EABs::Triangles]);
	glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(Triangle) * mesh->triangleCount, triangles);

    convexDesc.indices.count = mesh->triangleCount;
    convexDesc.indices.stride = sizeof(Triangle);
    convexDesc.indices.data = triangles;

    PxConvexMesh* convexMesh = nullptr;
    PxDefaultMemoryOutputStream buf;
    Physics& physics = Physics::Instance();
    if (physics.GetCooking().cookConvexMesh(convexDesc, buf)) {
        PxDefaultMemoryInputData id(buf.getData(), buf.getSize());
        convexMesh = physics.GetApi().createConvexMesh(id);
    }

	delete[] vertices;
	delete[] triangles;

    InitializeGeometry(convexMesh);
}

void ConvexMeshCollider::InitializeGeometry(PxConvexMesh* mesh) {
    InitializeRenderMesh(mesh);
	PxMeshScale scale(Transform::ToPx(transform.GetGlobalScale()));
    geometry = new PxConvexMeshGeometry(mesh, scale);
}

void ConvexMeshCollider::InitializeRenderMesh(PxConvexMesh* convexMesh) {
    const PxU32 polygonCount = convexMesh->getNbPolygons();
    const PxVec3 *convexVertices = convexMesh->getVertices();
    const PxU8 *indexBuffer = convexMesh->getIndexBuffer();

    std::vector<glm::vec3> vertices;
    std::vector<Triangle> triangles;

    for (PxU32 i = 0; i < polygonCount; ++i) {
        PxHullPolygon face;
        bool status = convexMesh->getPolygonData(i, face);
        PX_ASSERT(status);

        const size_t offset = vertices.size();
        const PxU8 *faceIndices = indexBuffer + face.mIndexBase;
        for (PxU32 j = 0; j < face.mNbVerts; ++j) {
            vertices.push_back(Transform::FromPx(convexVertices[faceIndices[j]]));
        }

        for (PxU32 j = 2; j < face.mNbVerts; ++j) {
            const Triangle triangle(
                unsigned short(offset),
                unsigned short(offset + j),
                unsigned short(offset + j - 1)
            );
            
            triangles.push_back(triangle);
        }
    }

    renderMesh = new Mesh(triangles.size(), vertices.size(), triangles.data(), vertices.data());
}
