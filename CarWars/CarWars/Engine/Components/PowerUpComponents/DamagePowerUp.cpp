#pragma once
#include "DamagePowerUp.h"

DamagePowerUp::DamagePowerUp() {
    Entity* powerUp = EntityManager::CreateDynamicEntity();
    EntityManager::SetTag(powerUp, "PowerUp");
    MeshComponent* powerUpMesh = new MeshComponent("Sphere.obj", "Basic.json","Boulder.jpg");
    /*powerUp->transform.SetPosition(EntityManager::FindEntities("DamagePowerUp")[0]->transform.GetGlobalPosition());*/
    powerUp->transform.SetPosition(glm::vec3(-70.f, 2.f, -70.f));
    powerUp->transform.SetScale(glm::vec3(1.f, 1.f, 1.f));

    EntityManager::AddComponent(powerUp, powerUpMesh);
    EntityManager::AddComponent(powerUp, this);

    RigidDynamicComponent* powerUpRigid = new RigidDynamicComponent();
    EntityManager::AddComponent(powerUp, powerUpRigid);
    PxMaterial* material = ContentManager::GetPxMaterial("Default.json");
    BoxCollider* powerUpCollider = new BoxCollider("PowerUps", material, PxFilterData(), glm::vec3(2.f, 2.f, 2.f));
    powerUpRigid->AddCollider(powerUpCollider);
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