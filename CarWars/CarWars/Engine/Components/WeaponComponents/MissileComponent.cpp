#include "MissileComponent.h"

#include "../../Systems/Game.h"
#include "../../Systems/Physics.h"
#include "../Colliders/BoxCollider.h"
#include "../../Entities/Transform.h"
#include "../../Entities/EntityManager.h"
#include "../../Components/CameraComponent.h"
#include "../../Systems/Content/\ContentManager.h"
#include "../../Components/WeaponComponents/WeaponComponent.h"
#include "../../Components/RigidbodyComponents/RigidDynamicComponent.h"

MissileComponent::MissileComponent() {}

void MissileComponent::Initialize(Entity* _entity, glm::vec3 position) {
	//Set Owner
	owner = _entity;
	//Get Its Damage Ammount
	damage = owner->GetComponent<WeaponComponent>()->GetDamage();
	Entity* missile = GetEntity();
	Entity* gunTurret = EntityManager::FindFirstChild(owner, "GunTurret");
	RigidDynamicComponent* missileRigidDynamic = missile->GetComponent<RigidDynamicComponent>();

	//Variables Needed
	glm::vec3 gunPosition = gunTurret->transform.GetGlobalPosition();
	glm::vec3 gunDirection = glm::normalize(position - gunPosition);

	float missileSpeed = missile->GetComponent<MissileComponent>()->GetSpeed();

	//Calculate Missile Orientation
	glm::vec3 a = glm::cross(-missile->transform.GetForward(), gunDirection);
	glm::quat q(0.f, a);
	q.w = 1 + glm::dot(-missile->transform.GetForward(), gunDirection);

	glm::vec3 missilePos = gunPosition;
	glm::quat missileRotation =  owner->transform.GetLocalRotation() * gunTurret->transform.GetLocalRotation();
	Transform missileTransform = Transform(nullptr, missilePos, glm::vec3(1), glm::normalize(q), false);

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