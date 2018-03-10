#include "CollisionCallback.h"
#include "../Physics.h"
#include "../../Components/RigidbodyComponents/VehicleComponent.h"
#include "../Engine/Components/WeaponComponents/MissileComponent.h"

void HandleMissileCollision(Entity* _actor0, Entity* _actor1) {
	if (_actor0->HasTag("Missile")) {
		if (_actor1->HasTag("Vehicle") || _actor1->HasTag("AiVehicle")) {
			if (_actor1->GetId() == _actor0->GetComponent<MissileComponent>()->GetOwner()->GetId()) {
				//Dont Explode
				std::cout << "Missile Inside Owner" << std::endl;
			} else {
				//Explode
				float damage = _actor0->GetComponent<MissileComponent>()->GetDamage();
				_actor1->GetComponent<VehicleComponent>()->TakeDamage(damage);
				Physics::Instance().AddToDelete(_actor0);
			}
		} else {
			Physics::Instance().AddToDelete(_actor0);
			//Explode
		}
	}
}

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