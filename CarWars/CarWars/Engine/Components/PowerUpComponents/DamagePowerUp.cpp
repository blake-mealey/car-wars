#pragma once
#include "DamagePowerUp.h"

DamagePowerUp::DamagePowerUp() {
    Entity* powerUp = EntityManager::CreateDynamicEntity();
    EntityManager::SetTag(powerUp, "PowerUp");
    MeshComponent* powerUpMesh = new MeshComponent("Sphere.obj", "Basic.json","Boulder.jpg");

    EntityManager::AddComponent(powerUp, powerUpMesh);

    RigidDynamicComponent* powerUpRigid = new RigidDynamicComponent();
    EntityManager::AddComponent(powerUp, powerUpRigid);
    PxMaterial* material = ContentManager::GetPxMaterial("Default.json");
    BoxCollider* powerUpCollider = new BoxCollider("PowerUps", material, PxFilterData(), glm::vec3(1.f, 1.f, 1.f));
    powerUpRigid->AddCollider(powerUpCollider);

}

void DamagePowerUp::Collect() {
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