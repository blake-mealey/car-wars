#include "CollisionCallback.h"
#include "../Physics.h"
#include "../../Components/PowerUpComponents/PowerUp.h"
#include "../../Components/PowerUpComponents/DamagePowerUp.h"
#include "../../Components/RigidbodyComponents/VehicleComponent.h"
#include "../Engine/Components/WeaponComponents/MissileComponent.h"
#include "../../Components/WeaponComponents/RocketLauncherComponent.h"
#include "../../Entities/Transform.h"
#include "../../Systems/Physics.h"
#include "../../Systems/Audio.h"
#include <vector>
#include "../Effects.h"
#include "PennerEasing/Quint.h"
#include "../../Components/RigidbodyComponents/RigidStaticComponent.h"

void HandlePowerUpCollision(Entity* _actor0, Entity* _actor1) {
    Entity* actor0;
    Entity* actor1;
    if (_actor0 && _actor1) {
        if (_actor0->HasTag("PowerUp")) {
            actor0 = _actor0;
            actor1 = _actor1;
        } else {
            actor0 = _actor1;
            actor1 = _actor0;
        }

        VehicleComponent* vehicle = actor1->GetComponent<VehicleComponent>();
        if (vehicle) {
            Physics& physicsInstance = Physics::Instance();
            actor0->GetComponent<PowerUp>()->Collect(actor1);
            physicsInstance.AddToDelete(actor0);
        }
    }
}

void HandleMissileCollision(Entity* _actor0, Entity* _actor1) {
	if (_actor0->HasTag("Missile")) {
		Physics& physicsInstance = Physics::Instance();
		if (_actor1->GetId() == _actor0->GetComponent<MissileComponent>()->GetOwner()->GetId()) {
			//Dont Explode
			std::cout << "Missile Inside Owner - Do Not Explode" << std::endl;
		} else {
			//Explode
            const char *explosionSound = "Content/Sounds/explosion.mp3";
            glm::vec3 pos = _actor0->transform.GetGlobalPosition();
            std::cout << to_string(pos) << std::endl;
			float explosionRadius = _actor0->GetComponent<MissileComponent>()->GetExplosionRadius();
            Audio::Instance().PlayAudio2D(explosionSound); // TODO: is this the best place for this?
            //Audio::Instance().PlayAudio3D(explosionSound, pos, glm::vec3(0.f, 0.f, 0.f));
            Entity* explosionEffect = ContentManager::LoadEntity("ExplosionEffect.json");
            explosionEffect->transform.SetPosition(_actor0->transform.GetGlobalPosition());
            MeshComponent* mesh = explosionEffect->GetComponent<MeshComponent>();
            Material* mat = mesh->GetMaterial();
            
            auto tween = Effects::Instance().CreateTween<float, easing::Quint::easeOut>(0.f, 1.f, 0.5, StateManager::gameTime);
            tween->SetUpdateCallback([mesh, mat, explosionRadius](float& value) mutable {
                if (StateManager::GetState() != GameState_Playing) return;
                mesh->transform.SetScale(glm::mix(glm::vec3(0.f), glm::vec3(explosionRadius*0.5f), value));
                mat->diffuseColor = glm::mix(glm::vec4(1.f, 0.f, 0.f, 1.f), glm::vec4(1.f, 0.f, 0.f, 0.f), value);
                mat->specularColor = glm::mix(glm::vec4(1.f, 0.f, 0.f, 1.f), glm::vec4(1.f, 0.f, 0.f, 0.f), value);
            });
            tween->SetFinishedCallback([explosionEffect](float& value) mutable {
                if (StateManager::GetState() != GameState_Playing) return;
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
					std::cout << damageToTake << std::endl;
					component->TakeDamage(weapon, damageToTake);
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
    if (actor0 && actor1 && pairs->status != physx::PxPairFlag::eNOTIFY_TOUCH_LOST) {
        if (actor0->HasTag("Missile") || actor1->HasTag("Missle")) {
            HandleMissileCollision(actor0, actor1);
            HandleMissileCollision(actor1, actor0);
        }
        else if (actor0->HasTag("PowerUp") || actor1->HasTag("PowerUp")) {
            HandlePowerUpCollision(actor0, actor1);
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
        VehicleComponent* vehicle0 = actor0->GetComponent<VehicleComponent>();
        VehicleComponent* vehicle1 = actor1->GetComponent<VehicleComponent>();
        if (vehicle0 && vehicle1) {
            Audio::Instance().PlayAudio("Content/Sounds/car-on-car2.mp3");
        } else if (vehicle1 || vehicle0) {
            Audio::Instance().PlayAudio("Content/Sounds/car-on-car.mp3");
        }
    }

	/*if (actor0)
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
	if (actor0 && actor1) {
		HandleMissileCollision(actor0, actor1);
		HandleMissileCollision(actor1, actor0);
	}*/
}
