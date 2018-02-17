#include "BoxCollider.h"
#include <cooking/PxConvexMeshDesc.h>
#include "../../Entities/Transform.h"
#include "../../Systems/Content/ContentManager.h"

using namespace physx;

BoxCollider::BoxCollider(std::string _collisionGroup, physx::PxMaterial *_material, glm::vec3 _size) : Collider(_collisionGroup, _material), size(_size) {
    BoxCollider::InitializeGeometry();
}

BoxCollider::BoxCollider(nlohmann::json data) : Collider(data) {
    size = ContentManager::JsonToVec3(data["Size"], glm::vec3(1.f));
    BoxCollider::InitializeGeometry();
}

void BoxCollider::InitializeGeometry() {
    geometry = new PxBoxGeometry(Transform::ToPx(size) * 0.5f);
}
