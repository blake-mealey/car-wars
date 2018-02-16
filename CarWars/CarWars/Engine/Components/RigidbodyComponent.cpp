#include "RigidbodyComponent.h"
#include <PxRigidDynamic.h>
#include "../Systems/Physics.h"

RigidbodyComponent::RigidbodyComponent() {
    physx::PxRigidDynamic* pxRigid = Physics::Instance().GetApi()->createRigidDynamic(PxTransform(PxIdentity));

}

ComponentType RigidbodyComponent::GetType() {
	return ComponentType_Rigidbody;
}

void RigidbodyComponent::HandleEvent(Event *event) {}