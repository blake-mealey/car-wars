#include "CollisionCallback.h"
#include "../Physics.h"
#include "../../Components/RigidbodyComponents/VehicleComponent.h"
#include "../Engine/Components/WeaponComponents/MissileComponent.h"
#include "../../Components/WeaponComponents/RocketLauncherComponent.h"

void HandleMissileCollision(Entity* _actor0, Entity* _actor1) {
	if (_actor0->HasTag("Missile")) {
		if (_actor1->HasTag("Vehicle") || _actor1->HasTag("AiVehicle")) {
			if (_actor1->GetId() == _actor0->GetComponent<MissileComponent>()->GetOwner()->GetId()) {
				//Dont Explode
				std::cout << "Missile Inside Owner" << std::endl;
			} else {
				//Explode
                RocketLauncherComponent* weapon = _actor0->GetComponent<MissileComponent>()->GetOwner()->GetComponent<RocketLauncherComponent>();
				_actor1->GetComponent<VehicleComponent>()->TakeDamage(weapon);
                Physics::Instance().AddToDelete(_actor0);
			}
		} else {
            Physics::Instance().AddToDelete(_actor0);
		}
	}
}

void CollisionCallback::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) {
	Physics& physicsInstance = Physics::Instance();
	RigidbodyComponent* actor0RB = static_cast<RigidbodyComponent*>(pairs->triggerActor->userData);
	Entity* actor0 = actor0RB->GetEntity();
	RigidbodyComponent* actor1RB = static_cast<RigidbodyComponent*>(pairs->otherActor->userData);
	Entity* actor1 = actor1RB->GetEntity();

	if (actor0 && actor1) {
		HandleMissileCollision(actor0, actor1);
		HandleMissileCollision(actor1, actor0);
	}
}

/*
void CollisionCallback::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) {
	Physics& physicsInstance = Physics::Instance();
	RigidbodyComponent* actor0RB = static_cast<RigidbodyComponent*>(pairHeader.actors[0]->userData);
	Entity* actor0 = actor0RB->GetEntity();
	RigidbodyComponent* actor1RB = static_cast<RigidbodyComponent*>(pairHeader.actors[1]->userData);
	Entity* actor1 = actor1RB->GetEntity();

	if (actor0 && actor1) {
		HandleMissileCollision(actor0, actor1);
		HandleMissileCollision(actor1, actor0);
	}
}
*/
