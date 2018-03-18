#include "CollisionCallback.h"
#include "../Physics.h"
#include "../../Components/PowerUpComponents/PowerUp.h"
#include "../../Components/PowerUpComponents/DamagePowerUp.h"
#include "../../Components/RigidbodyComponents/VehicleComponent.h"
#include "../Engine/Components/WeaponComponents/MissileComponent.h"
#include "../../Components/WeaponComponents/RocketLauncherComponent.h"
#include "../../Entities/Transform.h"
#include "../../Systems/Physics.h"
#include <vector>

//<<<<<<< HEAD
//
//void CollisionCallback::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) {
//    Physics& physicsInstance = Physics::Instance();
//
//    for (PxU32 i = 0; i < count; i++) {
//        // ignore pairs when shapes have been deleted
//        if (pairs[i].flags & (PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER |
//            PxTriggerPairFlag::eREMOVED_SHAPE_OTHER)) {
//            continue;
//        }
//        auto actorRB = static_cast<RigidbodyComponent*>(pairs[i].triggerShape->getActor()->userData);
//        auto actor = actorRB->GetEntity();
//        if (actor->HasTag("PowerUp")) {
//            actor->GetComponent<DamagePowerUp>()->Collect();
//            physicsInstance.AddToDelete(actor);
//        }
//    }
//}
//
//=======

void HandleMissileCollision(Entity* _actor0, Entity* _actor1) {
	if (_actor0->HasTag("Missile")) {
		Physics& physicsInstance = Physics::Instance();
		if (_actor1->GetId() == _actor0->GetComponent<MissileComponent>()->GetOwner()->GetId()) {
			//Dont Explode
			std::cout << "Missile Inside Owner - Do Not Explode" << std::endl;
		} else {
			//Explode
			float explosionRadius = _actor0->GetComponent<MissileComponent>()->GetExplosionRadius();

			//bool isOverlapping = overlap();
			//Entity* explosionEntity;
			Entity* explosionEntity = ContentManager::LoadEntity("Explosion.json");

			explosionEntity->GetComponent<RigidDynamicComponent>()->actor->setGlobalPose(_actor0->GetComponent<RigidDynamicComponent>()->actor->getGlobalPose());
			explosionEntity->GetComponent<RigidDynamicComponent>()->actor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);

			std::vector<Component*> carComponents = EntityManager::GetComponents(ComponentType_Vehicle);
			for (Component* component : carComponents) {
				if (glm::length(component->GetEntity()->transform.GetGlobalPosition() - _actor0->transform.GetGlobalPosition()) < explosionRadius) {
					RocketLauncherComponent* weapon = _actor0->GetComponent<MissileComponent>()->GetOwner()->GetComponent<RocketLauncherComponent>();
					//Take Damage Equal to damage / 1 + distanceFromExplosion?
					component->TakeDamage(weapon);
				}
			}
			physicsInstance.AddToDelete(_actor0);
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
>>>>>>> master
void CollisionCallback::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) {
	Physics& physicsInstance = Physics::Instance();
	RigidbodyComponent* actor0RB = static_cast<RigidbodyComponent*>(pairHeader.actors[0]->userData);
	Entity* actor0 = actor0RB->GetEntity();
	RigidbodyComponent* actor1RB = static_cast<RigidbodyComponent*>(pairHeader.actors[1]->userData);
	Entity* actor1 = actor1RB->GetEntity();

<<<<<<< HEAD
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
=======
	if (actor0 && actor1) {
		HandleMissileCollision(actor0, actor1);
		HandleMissileCollision(actor1, actor0);
	}
}
*/
