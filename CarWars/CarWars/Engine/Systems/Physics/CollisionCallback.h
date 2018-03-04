#pragma once

#include <iostream>

#include <PxSimulationEventCallback.h>

#include "../Engine/Entities/EntityManager.h"
#include "../Engine/Components/RigidbodyComponents/RigidbodyComponent.h"

class CollisionCallback : public physx::PxSimulationEventCallback {
public:

	void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override {};
	void onWake(physx::PxActor** actors, physx::PxU32 count) override {};
	void onSleep(physx::PxActor **actors, physx::PxU32 count) override {};
	void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override {};

	void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override {
		if (static_cast<RigidbodyComponent*>(pairHeader.actors[0]->userData)->GetEntity()->HasTag("Missile")) {
			EntityManager::DestroyDynamicEntity(static_cast<RigidbodyComponent*>(pairHeader.actors[0]->userData)->GetEntity());
		} else if (static_cast<RigidbodyComponent*>(pairHeader.actors[1]->userData)->GetEntity()->HasTag("Missile")) {
			EntityManager::DestroyDynamicEntity(static_cast<RigidbodyComponent*>(pairHeader.actors[1]->userData)->GetEntity());
		}
	}

	void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override {};

private:

};