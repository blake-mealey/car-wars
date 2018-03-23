#pragma once
#include "SpeedPowerUp.h"
#include "../../Systems/Effects.h"
#include "../Dependencies/PennerEasing/Quint.h"
#include "../../Components/GuiComponents/GuiComponent.h"
#include "../../Components/RigidbodyComponents/VehicleComponent.h"

SpeedPowerUp::SpeedPowerUp() {}

void SpeedPowerUp::Collect() {
    std::cout << "Speed Collected" << std::endl;
}

void SpeedPowerUp::SetEntity(Entity* _entity) {
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

void SpeedPowerUp::Collect(Entity* car) {
    PowerUp::Collect();
    PlayerData* player = Game::Instance().GetPlayerFromEntity(car);
    VehicleComponent* vehicle = car->GetComponent<VehicleComponent>();
    vehicle->speedMutliplier = 1.25f;
    if (player) {
        Entity* guiRoot = player->camera->GetGuiRoot();
        Entity* guiEntity = EntityManager::FindFirstChild(guiRoot, "SpeedPowerUp");
        GuiComponent* gui = guiEntity->GetComponent<GuiComponent>();
        auto tween = Effects::Instance().CreateTween<float, easing::Quint::easeOut>(0.f, 1.f, 0.25, StateManager::gameTime);
        tween->SetUpdateCallback([gui](float &value) mutable {
            gui->SetTextureOpacity(value);
            //gui->SetTextureColor(glm::vec4(1.f, 1.f, 0.f, value));
            gui->transform.SetScale(glm::mix(glm::vec3(100.f, 100.f, 0.f), glm::vec3(0.f, 0.f, 0.f), value));
        });
        tween->Start();
    }
}

ComponentType SpeedPowerUp::GetType() {
    return ComponentType_SpeedPowerUp;
}

void SpeedPowerUp::HandleEvent(Event *event) {
    return;
}

void SpeedPowerUp::RenderDebugGui() {
    PowerUp::RenderDebugGui();
}