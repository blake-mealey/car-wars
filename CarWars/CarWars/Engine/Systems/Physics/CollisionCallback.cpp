#include "CollisionCallback.h"
#include "../Physics.h"
#include "../Engine/Components/WeaponComponents/MissileComponent.h"
#include "../../Components/WeaponComponents/RocketLauncherComponent.h"
#include "../../Entities/Transform.h"
#include <vector>
#include "../Effects.h"
#include "PennerEasing/Quint.h"
#include "../../Components/RigidbodyComponents/RigidStaticComponent.h"

void HandleMissileCollision(Entity* _actor0, Entity* _actor1) {
	if (_actor0->HasTag("Missile")) {
		Physics& physicsInstance = Physics::Instance();
		if (_actor1->GetId() == _actor0->GetComponent<MissileComponent>()->GetOwner()->GetId()) {
			//Dont Explode
			std::cout << "Missile Inside Owner - Do Not Explode" << std::endl;
		} else {
			//Explode
			float explosionRadius = _actor0->GetComponent<MissileComponent>()->GetExplosionRadius();

            Entity* explosionEffect = ContentManager::LoadEntity("ExplosionEffect.json");
            explosionEffect->transform.SetPosition(_actor0->transform.GetGlobalPosition());
            MeshComponent* mesh = explosionEffect->GetComponent<MeshComponent>();
            Material* mat = mesh->GetMaterial();
            
            auto tween = Effects::Instance().CreateTween<float, easing::Quint::easeOut>(0.f, 1.f, 0.5);
            tween->SetUpdateCallback([mesh, mat, explosionRadius](float& value) mutable {
                if (StateManager::GetState() != GameState_Playing) return;
                mesh->transform.SetScale(glm::mix(glm::vec3(0.f), glm::vec3(explosionRadius*0.5f), value));
                mat->diffuseColor = glm::mix(glm::vec4(1.f, 0.f, 0.f, 1.f), glm::vec4(1.f, 0.f, 0.f, 0.f), value);
                mat->specularity = 1.f - value;
                mat->emissiveness = 1.f - value;
            });
            tween->SetFinishedCallback([explosionEffect](float& value) mutable {
                if (StateManager::GetState() != GameState_Playing) return;
                EntityManager::DestroyEntity(explosionEffect);
            });
            tween->Start();


			//bool isOverlapping = overlap();
			//Entity* explosionEntity;
//			Entity* explosionEntity = ContentManager::LoadEntity("Explosion.json");
//			explosionEntity->GetComponent<RigidStaticComponent>()->pxRigid->setGlobalPose(_actor0->GetComponent<RigidDynamicComponent>()->actor->getGlobalPose());

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
