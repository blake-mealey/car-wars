#include "CollisionCallback.h"
#include "../Physics.h"
#include "../../Components/RigidbodyComponents/VehicleComponent.h"
#include "../Engine/Components/WeaponComponents/MissileComponent.h"

void CollisionCallback::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) {
	Physics& physicsInstance = Physics::Instance();
	RigidbodyComponent* actor0RB = static_cast<RigidbodyComponent*>(pairHeader.actors[0]->userData);
	Entity* actor0 = actor0RB->GetEntity();
	RigidbodyComponent* actor1RB = static_cast<RigidbodyComponent*>(pairHeader.actors[1]->userData);
	Entity* actor1 = actor1RB->GetEntity();
	if (actor0)
		if (actor0->HasTag("Missile")) {
			physicsInstance.AddToDelete(actor0);
			if (actor1->HasTag("Vehicle")) {
				float damage = static_cast<MissileComponent*>(actor0->components[1])->GetDamage();
				static_cast<VehicleComponent*>(actor1RB)->TakeDamage(damage);
				std::cout << static_cast<VehicleComponent*>(actor1RB)->GetHealth() << std::endl;
			}
		}
	if (actor1)
		if (actor1->HasTag("Missile")) {
			physicsInstance.AddToDelete(actor1);
			if (actor0->HasTag("Vehicle")) {
				float damage = static_cast<MissileComponent*>(actor1->components[1])->GetDamage();
				static_cast<VehicleComponent*>(actor0RB)->TakeDamage(damage);
				std::cout << static_cast<VehicleComponent*>(actor0RB)->GetHealth() << std::endl;
			}
		}
	/*
	if (static_cast<RigidbodyComponent*>(pairHeader.actors[0]->userData)->GetEntity()->HasTag("Missile")) {
	//EntityManager::DestroyDynamicEntity(static_cast<RigidbodyComponent*>(pairHeader.actors[0]->userData)->GetEntity());
	} else if (static_cast<RigidbodyComponent*>(pairHeader.actors[1]->userData)->GetEntity()->HasTag("Missile")) {
	EntityManager::DestroyDynamicEntity(static_cast<RigidbodyComponent*>(pairHeader.actors[1]->userData)->GetEntity());
	}
	*/
}