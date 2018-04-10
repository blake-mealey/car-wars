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
#include "PennerEasing/Back.h"

void HandleMissileCollision(Entity* _actor0, Entity* _actor1) {
	if (_actor0->HasTag("Missile")) {
        MissileComponent* missile = _actor0->GetComponent<MissileComponent>();
		if (missile->enabled && _actor1->GetId() != missile->GetOwner()->GetId()) {
			//Explode
            const glm::vec3 pos = _actor0->transform.GetGlobalPosition();
			const float explosionRadius = missile->GetExplosionRadius();
            Audio::Instance().PlayAudio3D("Content/Sounds/explosion.mp3", pos, glm::vec3(0.f, 0.f, 0.f), 2.f);
            
		    Entity* explosionEffect = ContentManager::LoadEntity("ExplosionEffect.json");
            explosionEffect->transform.SetPosition(_actor0->transform.GetGlobalPosition());

            {
                PointLightComponent* light = EntityManager::FindFirstChild(explosionEffect, "Light")->GetComponent<PointLightComponent>();
                constexpr float power = 150.f;
                
                auto tweenIn = Effects::Instance().CreateTween<float, easing::Quint::easeOut>(0.f, power, 0.2, StateManager::gameTime);
                tweenIn->SetUpdateCallback([light](float& value) mutable {
                    light->SetPower(value);
                });

                auto tweenOut = Effects::Instance().CreateTween<float, easing::Quint::easeIn>(power, 0.f, 0.1, StateManager::gameTime);
                tweenOut->SetUpdateCallback([light](float& value) mutable {
                    light->SetPower(value);
                });

                tweenIn->SetNext(tweenOut);
                tweenIn->Start();
            }
            
		    ParticleEmitterComponent* emitter = explosionEffect->GetComponent<ParticleEmitterComponent>();
            auto tween = Effects::Instance().CreateTween<float, easing::Linear::easeNone>(0.f, 1.f, emitter->GetLifetimeSeconds(), StateManager::gameTime);
            tween->SetFinishedCallback([explosionEffect, _actor0](float& value) mutable {
                Physics::Instance().AddToDelete(_actor0);
                EntityManager::DestroyEntity(explosionEffect);
            });
            tween->Start();

			vector<Component*> carComponents = EntityManager::GetComponents(ComponentType_Vehicle);
			std::cout << carComponents.size() << std::endl;
			for (Component* component : carComponents) {
				if (glm::length(component->GetEntity()->transform.GetGlobalPosition() - _actor0->transform.GetGlobalPosition()) < explosionRadius) {
					RocketLauncherComponent* weapon = _actor0->GetComponent<MissileComponent>()->GetOwner()->GetComponent<RocketLauncherComponent>();
					//Take Damage Equal to damage / 1 + distanceFromExplosion?
					float missileDamage = _actor0->GetComponent<MissileComponent>()->GetDamage();
					float damageToTake = missileDamage - (15.0f * (glm::length(component->GetEntity()->transform.GetGlobalPosition() - _actor0->transform.GetGlobalPosition())));
					static_cast<VehicleComponent*>(component)->pxVehicle->getRigidDynamicActor()->addForce(Transform::ToPx(glm::normalize(component->GetEntity()->transform.GetGlobalPosition() - _actor0->transform.GetGlobalPosition()) * 20000.0f), PxForceMode::eIMPULSE, true);
					component->TakeDamage(weapon, damageToTake);
				}
			}

            missile->enabled = false;
            _actor0->GetComponent<MeshComponent>()->enabled = false;
			for (ParticleEmitterComponent* emitter : _actor0->GetComponents<ParticleEmitterComponent>()) {
				emitter->SetSpawnRate(0.f);
			}
			//Physics::Instance().AddToDelete(missile->GetEntity());
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
