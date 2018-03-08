#include "MissileComponent.h"

#include "../../Systems/Physics.h"
#include "../Colliders/BoxCollider.h"
#include "../../Entities/Transform.h"
#include "../../Entities/EntityManager.h"
#include "../../Systems/Content/\ContentManager.h"
#include "../../Components/WeaponComponents/WeaponComponent.h"
#include "../../Components/RigidbodyComponents/RigidDynamicComponent.h"

MissileComponent::MissileComponent() {}

void MissileComponent::Initialize(Entity* _entity) {
	owner = _entity;
	damage = owner->GetComponent<WeaponComponent>()->GetDamage();
	Entity* missile = GetEntity();
	RigidDynamicComponent* missileRigidDynamic = missile->GetComponent<RigidDynamicComponent>();
	Entity* gunTurret = EntityManager::FindFirstChild(owner, "GunTurret");

	constexpr float offsetFromGun = 5.0f;
	float missileSpeed = missile->GetComponent<MissileComponent>()->GetSpeed();

	glm::vec3 missilePos = gunTurret->transform.GetGlobalPosition() - gunTurret->transform.GetForward() * offsetFromGun;
	glm::quat missileRotation = owner->transform.GetLocalRotation() * gunTurret->transform.GetLocalRotation();
	Transform missileTransform = Transform(nullptr, missilePos, glm::vec3(1), missileRotation, false);

	missileRigidDynamic->actor->setGlobalPose(Transform::ToPx(missileTransform));
	missileRigidDynamic->actor->setLinearVelocity(Transform::ToPx(-missileTransform.GetForward() * missileSpeed), true);
	missileRigidDynamic->actor->setLinearDamping(0.0);
	missileRigidDynamic->actor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
}

float MissileComponent::GetSpeed() {
	return missileSpeed;
}

float MissileComponent::GetDamage() {
	return damage;
}

Entity* MissileComponent::GetOwner() {
	return owner;
}

ComponentType MissileComponent::GetType() {
	return ComponentType_Missile;
}

void MissileComponent::HandleEvent(Event *event){
	return;
}