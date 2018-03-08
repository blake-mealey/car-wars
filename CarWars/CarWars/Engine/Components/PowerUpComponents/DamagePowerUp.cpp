#pragma once
#include "DamagePowerUp.h"

DamagePowerUp::DamagePowerUp() {

}

void DamagePowerUp::SetEntity(Entity* _entity) {
    Component::SetEntity(_entity);
    RigidDynamicComponent* powerUpRigid = new RigidDynamicComponent();
    EntityManager::AddComponent(GetEntity(), powerUpRigid);
    PxMaterial* material = ContentManager::GetPxMaterial("Default.json");
    BoxCollider* powerUpCollider = new BoxCollider("PowerUps", material, PxFilterData(), glm::vec3(2.f, 2.f, 2.f));
    

    powerUpRigid->AddCollider(powerUpCollider);
    PxShape* shape;
    powerUpRigid->actor->getShapes(&shape, 1);
    shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
    shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
    powerUpRigid->actor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
}

void DamagePowerUp::Collect() {
    PowerUp::Collect();
    std::cout << "Damage Collected" << std::endl;
}

ComponentType DamagePowerUp::GetType() {
    return ComponentType_DamagePowerUp;
}

void DamagePowerUp::HandleEvent(Event *event) {
    return;
}

void DamagePowerUp::RenderDebugGui() {
    PowerUp::RenderDebugGui();
}