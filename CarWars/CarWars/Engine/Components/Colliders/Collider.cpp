#include "Collider.h"
#include <extensions/PxRigidActorExt.h>
#include "../../Systems/Physics/CollisionFilterShader.h"
#include "../../Systems/Content/ContentManager.h"

using namespace physx;

Collider::Collider(std::string _collisionGroup, physx::PxMaterial *_material) : collisionGroup(_collisionGroup), material(_material), shape(nullptr), geometry(nullptr) {}

Collider::Collider(nlohmann::json data) {
    collisionGroup = ContentManager::GetFromJson<std::string>(data["CollisionGroup"], "DrivableObstacles");
    material = ContentManager::GetPxMaterial(ContentManager::GetFromJson<std::string>(data["Material"], "Default.json"));
}

Collider::~Collider() {
    shape->release();
    delete geometry;
}

physx::PxShape* Collider::GetShape() const {
    return shape;
}

void Collider::CreateShape(PxRigidActor *actor) {
    shape = physx::PxRigidActorExt::createExclusiveShape(*actor, *geometry, *material);
    //shape->setQueryFilterData(queryFilterData);                                         // For raycasts
    shape->setSimulationFilterData(CollisionGroups::GetFilterData(collisionGroup));     // For collisions
    shape->setLocalPose(PxTransform(PxIdentity));
}
