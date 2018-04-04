#include "CollisionCallback.h"
#include "../Physics.h"
#include "../../Components/RigidbodyComponents/VehicleComponent.h"
#include "../Engine/Components/WeaponComponents/MissileComponent.h"
#include "../../Components/WeaponComponents/RocketLauncherComponent.h"
#include "../../Entities/Transform.h"
#include "../../Systems/Audio.h"
#include <vector>
#include "../Effects.h"
#include "PennerEasing/Quint.h"
#include "../../Components/RigidbodyComponents/RigidStaticComponent.h"
#include "PennerEasing/Linear.h"
#include "../../Components/ParticleEmitterComponent.h"

void HandleMissileCollision(Entity* _actor0, Entity* _actor1) {
	if (_actor0->HasTag("Missile")) {
		Physics& physicsInstance = Physics::Instance();
		if (_actor1->GetId() == _actor0->GetComponent<MissileComponent>()->GetOwner()->GetId()) {
		} else {
			//Explode
            glm::vec3 pos = _actor0->transform.GetGlobalPosition();
			float explosionRadius = _actor0->GetComponent<MissileComponent>()->GetExplosionRadius();
            //Audio::Instance().PlayAudio("Content/Sounds/explosion.mp3", 1.f);
            Audio::Instance().PlayAudio3D("Content/Sounds/explosion.mp3", pos, glm::vec3(0.f, 0.f, 0.f), 2.f);
            
		    Entity* explosionEffect = ContentManager::LoadEntity("ExplosionEffect.json");
            explosionEffect->transform.SetPosition(_actor0->transform.GetGlobalPosition());
            ParticleEmitterComponent* emitter = explosionEffect->GetComponent<ParticleEmitterComponent>();
            
            auto tween = Effects::Instance().CreateTween<float, easing::Linear::easeInOut>(0.f, 1.f, emitter->GetLifetimeSeconds(), StateManager::gameTime);
            tween->SetFinishedCallback([explosionEffect](float& value) mutable {
                EntityManager::DestroyEntity(explosionEffect);
            });
            tween->Start();


			std::vector<Component*> carComponents = EntityManager::GetComponents(ComponentType_Vehicle);
			for (Component* component : carComponents) {
				if (glm::length(component->GetEntity()->transform.GetGlobalPosition() - _actor0->transform.GetGlobalPosition()) < explosionRadius) {
					RocketLauncherComponent* weapon = _actor0->GetComponent<MissileComponent>()->GetOwner()->GetComponent<RocketLauncherComponent>();
					//Take Damage Equal to damage / 1 + distanceFromExplosion?
					float missileDamage = _actor0->GetComponent<MissileComponent>()->GetDamage();
					float damageToTake = missileDamage - (15.0f * (glm::length(component->GetEntity()->transform.GetGlobalPosition() - _actor0->transform.GetGlobalPosition())));
					component->TakeDamage(weapon, damageToTake);
				}
			}
			physicsInstance.AddToDelete(_actor0);
		}
	}
}

void CollisionCallback::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) {
    if (pairs->status != PxPairFlag::eNOTIFY_TOUCH_LOST) {
        RigidbodyComponent* actor0RB = static_cast<RigidbodyComponent*>(pairs->triggerActor->userData);
        Entity* actor0 = actor0RB->GetEntity();
        RigidbodyComponent* actor1RB = static_cast<RigidbodyComponent*>(pairs->otherActor->userData);
        Entity* actor1 = actor1RB->GetEntity();

        actor0RB->OnTrigger(actor1RB);
        actor1RB->OnTrigger(actor0RB);

        if (actor0->HasTag("Missile") || actor1->HasTag("Missle")) {
            HandleMissileCollision(actor0, actor1);
            HandleMissileCollision(actor1, actor0);
        }
	}
}

void CollisionCallback::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) {
    RigidbodyComponent* actor0RB = static_cast<RigidbodyComponent*>(pairHeader.actors[0]->userData);
    RigidbodyComponent* actor1RB = static_cast<RigidbodyComponent*>(pairHeader.actors[1]->userData);

    actor0RB->OnContact(actor1RB);
    actor1RB->OnContact(actor0RB);
}
