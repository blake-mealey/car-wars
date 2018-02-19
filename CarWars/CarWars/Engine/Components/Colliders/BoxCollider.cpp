#include "BoxCollider.h"
#include "../../Entities/Transform.h"
#include "../../Systems/Content/ContentManager.h"

using namespace physx;

BoxCollider::BoxCollider(std::string _collisionGroup, physx::PxMaterial *_material, physx::PxFilterData _queryFilterData, glm::vec3 _size)
    : Collider(_collisionGroup, _material, _queryFilterData), size(_size) {
    
    InitializeGeometry();
}

BoxCollider::BoxCollider(nlohmann::json data) : Collider(data) {
    size = ContentManager::JsonToVec3(data["Size"], glm::vec3(1.f));
    InitializeGeometry();
}

ColliderType BoxCollider::GetType() const {
    return Collider_Box;
}

Mesh* BoxCollider::GetRenderMesh() {
    return ContentManager::GetMesh("Cube.obj");
}

Transform BoxCollider::GetLocalTransform() const {
    Transform transform = Collider::GetLocalTransform();
    transform.SetScale(size * 0.5f);
    return transform;
}

Transform BoxCollider::GetGlobalTransform() const {
    Transform transform = Collider::GetGlobalTransform();
    transform.SetScale(size * 0.5f);
    return transform;
}

void BoxCollider::InitializeGeometry() {
    geometry = new PxBoxGeometry(Transform::ToPx(size) * 0.5f);
}