#include "MeshCollider.h"
#include <cooking/PxConvexMeshDesc.h>
#include "../../Systems/Content/Mesh.h"
#include "../../Systems/Physics.h"
#include "../../Systems/Content/ContentManager.h"
#include <iostream>

using namespace physx;

MeshCollider::~MeshCollider() {
    triangleMesh->release();
}

MeshCollider::MeshCollider(std::string _collisionGroup, physx::PxMaterial *_material, physx::PxFilterData _queryFilterData, bool _isTrigger, Mesh *_mesh)
    : Collider(_collisionGroup, _material, _queryFilterData, _isTrigger), fromHeightMap(false) {
    
    InitializeGeometry(_mesh);
}

MeshCollider::MeshCollider(nlohmann::json data) : Collider(data), fromHeightMap(false) {
    if (data["HeightMap"].is_string()) {
        fromHeightMap = true;
        HeightMap* map = ContentManager::GetHeightMap(data["HeightMap"]);
        InitializeGeometry(map->GetMesh());
    } else {
        InitializeGeometry(ContentManager::GetMesh(data["Mesh"]));
    }
}

ColliderType MeshCollider::GetType() const {
    return Collider_TriangleMesh;
}

Mesh* MeshCollider::GetRenderMesh() {
    return mesh;
}

void MeshCollider::InitializeGeometry(Mesh *renderMesh) {
	mesh = renderMesh;

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

    const PxCookingParams originalCookingParams = physics.GetCooking().getParams();
    PxCookingParams myCookingParams = originalCookingParams;
    if (fromHeightMap) myCookingParams.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
    physics.GetCooking().setParams(myCookingParams);

	triangleMesh = nullptr;
	PxDefaultMemoryOutputStream writeBuffer;
	PxTriangleMeshCookingResult::Enum result;
	if (physics.GetCooking().cookTriangleMesh(meshDesc, writeBuffer, &result)) {
		PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
		triangleMesh = physics.GetApi().createTriangleMesh(readBuffer);
	}
    
    physics.GetCooking().setParams(originalCookingParams);

	delete[] vertices;
	delete[] triangles;

	InitializeGeometry();
}

void MeshCollider::InitializeGeometry() {
	if (geometry != nullptr) delete geometry;
	PxMeshScale scale(Transform::ToPx(transform.GetGlobalScale()), PxQuat(PxIdentity));
    geometry = new PxTriangleMeshGeometry(triangleMesh, scale);
}
