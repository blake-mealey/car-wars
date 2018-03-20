#pragma once
#include "DamagePowerUp.h"
#include "../../Systems/Effects.h"
#include "../Dependencies/PennerEasing/Quint.h"
#include "../../Components/GuiComponents/GuiComponent.h"
#include "../../Components/WeaponComponents/WeaponComponent.h"

DamagePowerUp::DamagePowerUp() {

}

void DamagePowerUp::SetEntity(Entity* _entity) {
    Component::SetEntity(_entity);
    RigidDynamicComponent* powerUpRigid = new RigidDynamicComponent();
    EntityManager::AddComponent(GetEntity(), powerUpRigid);
    PxMaterial* material = ContentManager::GetPxMaterial("Default.json");
    BoxCollider* powerUpCollider = new BoxCollider("PowerUps", material, PxFilterData(), true, glm::vec3(2.f, 2.f, 2.f));
    

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

void DamagePowerUp::Collect(Entity* car) {
    PowerUp::Collect();
    PlayerData* player = Game::Instance().GetPlayerFromEntity(car);
    WeaponComponent* weapon = car->GetComponent<WeaponComponent>();
    weapon->damageMultiplier = 1.25f;
    if (player) {
        Entity* guiRoot = player->camera->GetGuiRoot();
        Entity* guiEntity = EntityManager::FindFirstChild(guiRoot, "PowerUp");
        GuiComponent* gui = guiEntity->GetComponent<GuiComponent>();
        auto tween = Effects::Instance().CreateTween<float,easing::Quint::easeOut>(0.f, 1.f,0.25);
        tween->SetUpdateCallback([gui](float &value) mutable {
            gui->SetTextureOpacity(value);
            gui->transform.SetScale(glm::mix(glm::vec3(100.f, 100.f, 0.f), glm::vec3(0.f, 0.f, 0.f), value));
        });
        tween->Start();
    }
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