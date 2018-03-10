#include "BoxCollider.h"
#include "../../Entities/Transform.h"
#include "../../Systems/Content/ContentManager.h"

using namespace physx;

BoxCollider::~BoxCollider() {}

BoxCollider::BoxCollider(std::string _collisionGroup, physx::PxMaterial *_material, physx::PxFilterData _queryFilterData, bool _isTrigger, glm::vec3 _scale)
    : Collider(_collisionGroup, _material, _queryFilterData, _isTrigger) {
    
	transform.SetScale(_scale);

    InitializeGeometry();
}

BoxCollider::BoxCollider(nlohmann::json data) : Collider(data) {
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
    transform.Scale(0.5f);
    return transform;
}

Transform BoxCollider::GetGlobalTransform() const {
    Transform transform = Collider::GetGlobalTransform();
    transform.Scale(0.5f);
    return transform;
}

void BoxCollider::InitializeGeometry() {
	if (geometry != nullptr) delete geometry;
    geometry = new PxBoxGeometry(Transform::ToPx(transform.GetLocalScale()) * 0.5f);
}
