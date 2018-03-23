#pragma once
#include "DamagePowerUp.h"
#include "../../Systems/Effects.h"
#include "../Dependencies/PennerEasing/Quint.h"
#include "../../Components/GuiComponents/GuiComponent.h"
#include "../../Components/WeaponComponents/WeaponComponent.h"
#include "../../Systems/Game.h"
#include "../Colliders/BoxCollider.h"
#include "../../Systems/Content/ContentManager.h"
#include "../RigidbodyComponents/RigidDynamicComponent.h"
#include "../../Entities/EntityManager.h"

DamagePowerUp::DamagePowerUp() {

}

void DamagePowerUp::Collect(PlayerData* player) {
    std::cout << "Damage Collected" << std::endl;
    PowerUp::Collect(player);

    WeaponComponent* weapon = player->vehicleEntity->GetComponent<WeaponComponent>();
    weapon->damageMultiplier = 1.25f;
    if (player) {
        Entity* guiRoot = player->camera->GetGuiRoot();
        Entity* guiEntity = EntityManager::FindFirstChild(guiRoot, "DamagePowerUp");
        GuiComponent* gui = guiEntity->GetComponent<GuiComponent>();
        auto tween = Effects::Instance().CreateTween<float,easing::Quint::easeOut>(0.f, 1.f,0.25, StateManager::gameTime);
        tween->SetUpdateCallback([gui](float &value) mutable {
            gui->SetTextureOpacity(value);
            gui->transform.SetScale(glm::mix(glm::vec3(100.f, 100.f, 0.f), glm::vec3(0.f, 0.f, 0.f), value));
        });
        tween->Start();
    }
}
