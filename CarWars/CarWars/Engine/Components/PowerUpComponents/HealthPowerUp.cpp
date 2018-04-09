#pragma once

#include "HealthPowerUp.h"
#include "../../Components/GuiComponents/GuiComponent.h"
#include "../../Components/RigidbodyComponents/VehicleComponent.h"
#include "../../Systems/Content/ContentManager.h"
#include "../../Systems/Game.h"

HealthPowerUp::HealthPowerUp() {}

void HealthPowerUp::Collect(PlayerData* player) {
    PowerUp::Collect(player);

    VehicleComponent* vehicle = player->vehicleEntity->GetComponent<VehicleComponent>();
    vehicle->AddHealth(250.f);
    HumanData *human = Game::GetHumanFromEntity(vehicle->GetEntity());
    if (human != nullptr)
        vehicle->UpdateHealthGui(human);

}

void HealthPowerUp::RemoveInternal() {
    //VehicleComponent* vehicle = player->vehicleEntity->GetComponent<VehicleComponent>();
}

std::string HealthPowerUp::GetGuiName() const {
    return "HealthPowerUp";
}

glm::vec4 HealthPowerUp::GetColor() const {
    return glm::vec4(0.f, 1.f, 0.f, 1.f);
}
