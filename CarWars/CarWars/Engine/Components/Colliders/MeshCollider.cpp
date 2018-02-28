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
	mesh = ContentManager::GetMesh(data["Mesh"]);
	
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

	glm::vec3 *vertices = new glm::vec3[mesh->vertexCount];
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbos[VBOs::Vertices]);
	glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * mesh->vertexCount, vertices);

    meshDesc.points.count = mesh->vertexCount;
    meshDesc.points.stride = sizeof(glm::vec3);
    meshDesc.points.data = vertices;

	glm::vec3 *triangles = new glm::vec3[mesh->triangleCount];
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->eabs[EABs::Triangles]);
	glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(Triangle) * mesh->triangleCount, triangles);

    meshDesc.triangles.count = mesh->triangleCount;
    meshDesc.triangles.stride = sizeof(Triangle);
    meshDesc.triangles.data = triangles;

    Physics& physics = Physics::Instance();

	PxTriangleMesh *triangleMesh = nullptr;
    PxDefaultMemoryOutputStream writeBuffer;
    PxTriangleMeshCookingResult::Enum result;
	if (physics.GetCooking().cookTriangleMesh(meshDesc, writeBuffer, &result)) {
		PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
		triangleMesh = physics.GetApi().createTriangleMesh(readBuffer);
	}

	delete[] vertices;
	delete[] triangles;

	PxMeshScale scale(Transform::ToPx(transform.GetGlobalScale()), PxQuat(PxIdentity));
    geometry = new PxTriangleMeshGeometry(triangleMesh, scale);
}
