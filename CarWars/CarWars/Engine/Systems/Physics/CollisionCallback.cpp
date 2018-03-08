#include "CollisionCallback.h"
#include "../Physics.h"
#include "../../Components/PowerUpComponents/PowerUp.h"
#include "../../Components/PowerUpComponents/DamagePowerUp.h"
#include "../../Components/RigidbodyComponents/VehicleComponent.h"
#include "../Engine/Components/WeaponComponents/MissileComponent.h"


void CollisionCallback::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) {
    Physics& physicsInstance = Physics::Instance();

    for (PxU32 i = 0; i < count; i++) {
        // ignore pairs when shapes have been deleted
        if (pairs[i].flags & (PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER |
            PxTriggerPairFlag::eREMOVED_SHAPE_OTHER)) {
            continue;
        }
        auto actorRB = static_cast<RigidbodyComponent*>(pairs[i].triggerShape->getActor()->userData);
        auto actor = actorRB->GetEntity();
        if (actor->HasTag("PowerUp")) {
            actor->GetComponent<DamagePowerUp>()->Collect();
            physicsInstance.AddToDelete(actor);
        }
    }
}

void CollisionCallback::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) {
	Physics& physicsInstance = Physics::Instance();
	RigidbodyComponent* actor0RB = static_cast<RigidbodyComponent*>(pairHeader.actors[0]->userData);
	Entity* actor0 = actor0RB->GetEntity();
	RigidbodyComponent* actor1RB = static_cast<RigidbodyComponent*>(pairHeader.actors[1]->userData);
	Entity* actor1 = actor1RB->GetEntity();

	if (actor0)
		if (actor0->HasTag("Missile")) {
			physicsInstance.AddToDelete(actor0);
			if (actor1->HasTag("AiVehicle")) {
				float damage = actor0->GetComponent<MissileComponent>()->GetDamage();
				static_cast<VehicleComponent*>(actor1RB)->TakeDamage(damage);
				std::cout << static_cast<VehicleComponent*>(actor1RB)->GetHealth() << std::endl;
			}
		}
	if (actor1)
		if (actor1->HasTag("Missile")) {
			physicsInstance.AddToDelete(actor1);
			if (actor0->HasTag("AiVehicle")) {
				float damage = actor1->GetComponent<MissileComponent>()->GetDamage();
				static_cast<VehicleComponent*>(actor0RB)->TakeDamage(damage);
				std::cout << static_cast<VehicleComponent*>(actor0RB)->GetHealth() << std::endl;
			}
		}
}