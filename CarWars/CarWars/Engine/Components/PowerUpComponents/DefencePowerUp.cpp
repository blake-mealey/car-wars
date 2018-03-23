#pragma once
#include "DefencePowerUp.h"
#include "../../Systems/Effects.h"
#include "../Dependencies/PennerEasing/Quint.h"
#include "../../Components/GuiComponents/GuiComponent.h"
#include "../../Components/RigidbodyComponents/VehicleComponent.h"
#include "../Colliders/BoxCollider.h"
#include "../../Systems/Content/ContentManager.h"
#include "../../Systems/Game.h"
#include "../../Entities/EntityManager.h"

DefencePowerUp::DefencePowerUp() {}


void DefencePowerUp::SetEntity(Entity* _entity) {
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

void DefencePowerUp::Collect(Entity* car) {
    std::cout << "Defence Collected" << std::endl;

    PowerUp::Collect(car);

    PlayerData* player = Game::Instance().GetPlayerFromEntity(car);
    VehicleComponent* vehicle = car->GetComponent<VehicleComponent>();
    vehicle->defenceMultiplier = 1.25f;
    if (player) {
        Entity* guiRoot = player->camera->GetGuiRoot();
        Entity* guiEntity = EntityManager::FindFirstChild(guiRoot, "DefencePowerUp");
        GuiComponent* gui = guiEntity->GetComponent<GuiComponent>();
        auto tween = Effects::Instance().CreateTween<float, easing::Quint::easeOut>(0.f, 1.f, 0.25, StateManager::gameTime);
        tween->SetUpdateCallback([gui](float &value) mutable {
            gui->SetTextureOpacity(value);
            gui->transform.SetScale(glm::mix(glm::vec3(100.f, 100.f, 0.f), glm::vec3(0.f, 0.f, 0.f), value));
        });
        tween->Start();
    }
}

ComponentType DefencePowerUp::GetType() {
    return ComponentType_DefencePowerUp;
}

void DefencePowerUp::RenderDebugGui() {
    PowerUp::RenderDebugGui();
}