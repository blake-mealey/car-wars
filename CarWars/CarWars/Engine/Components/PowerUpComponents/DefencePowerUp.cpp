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

void DefencePowerUp::Collect(PlayerData* player) {
    std::cout << "Defence Collected" << std::endl;

    PowerUp::Collect(player);

    VehicleComponent* vehicle = player->vehicleEntity->GetComponent<VehicleComponent>();
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
