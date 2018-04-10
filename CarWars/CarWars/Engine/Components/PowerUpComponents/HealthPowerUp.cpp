#pragma once

#include "HealthPowerUp.h"

HealthPowerUp::HealthPowerUp() { }

void updateGui(VehicleComponent* vehicle) {
    HumanData *human = Game::GetHumanFromEntity(vehicle->GetEntity());
    if (human != nullptr)
        vehicle->UpdateHealthGui(human);
}

void HealthPowerUp::Collect(PlayerData* player) {
    PowerUp::Collect(player);

    VehicleComponent* vehicle = player->vehicleEntity->GetComponent<VehicleComponent>();
    vehicle->AddHealth(50.f);
    updateGui(vehicle);
    collectedAt = StateManager::gameTime.GetSeconds();
}

void HealthPowerUp::Tick(PlayerData* player) {
    if ((StateManager::gameTime.GetSeconds() - collectedAt) > 1.f && numTicks > 0) {
        VehicleComponent* vehicle = player->vehicleEntity->GetComponent<VehicleComponent>();
        collectedAt += 1.f;
        numTicks--;
        vehicle->AddHealth(50.f);
        updateGui(vehicle);
    }
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
