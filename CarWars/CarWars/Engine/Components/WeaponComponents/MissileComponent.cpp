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

void MissileComponent::Initialize(Entity* _entity) {
	//Set Owner
	owner = _entity;
	//Get Its Damage Ammount
	damage = owner->GetComponent<WeaponComponent>()->GetDamage();
	Entity* missile = GetEntity();
	Entity* gunTurret = EntityManager::FindFirstChild(owner, "GunTurret");
	RigidDynamicComponent* missileRigidDynamic = missile->GetComponent<RigidDynamicComponent>();

	//Determine Player and Get Camera
	CameraComponent* vehicleCamera = nullptr;
	glm::vec3 cameraDirection;
	Game& gameInstance = Game::Instance();
	for (int i = 0; i < gameInstance.gameData.playerCount; ++i) {
		PlayerData& player = gameInstance.players[i];
		if (player.vehicleEntity->GetId() == owner->GetId()) {
			if (player.camera) {
				vehicleCamera = player.camera;
				cameraDirection = player.camera->GetTarget() - player.camera->GetPosition();
			}
		}
	}

	//Load Scene
	PxScene* scene = &Physics::Instance().GetScene();
	float rayLength = 100.0f;
	//Cast Camera Ray
	PxRaycastBuffer cameraHit;
	glm::vec3 cameraHitPosition;
	if (scene->raycast(Transform::ToPx(vehicleCamera->GetTarget()), Transform::ToPx(cameraDirection), rayLength, cameraHit)) {
		//cameraHit has hit something
		if (cameraHit.hasAnyHits()) {
			cameraHitPosition = Transform::FromPx(cameraHit.block.position);
		} else {
			//cameraHit has not hit anything
			cameraHitPosition = vehicleCamera->GetTarget() + (cameraDirection * rayLength);
		}
	}

	//Variables Needed
	glm::vec3 gunPosition = gunTurret->transform.GetGlobalPosition();
	glm::vec3 gunDirection = glm::normalize(cameraHitPosition - gunPosition);

	float missileSpeed = missile->GetComponent<MissileComponent>()->GetSpeed();

	//Calculate Missile Orientation
	glm::vec3 a = glm::cross(-missile->transform.GetForward(), gunDirection);
	glm::quat q(0.f, a);
	q.w = 1 + glm::dot(-missile->transform.GetForward(), gunDirection);

	//TODO: FIX MISSILE ROTATION - BASED ON gunDirection
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