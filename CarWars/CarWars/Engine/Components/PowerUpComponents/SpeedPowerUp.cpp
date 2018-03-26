#pragma once

#include "SpeedPowerUp.h"
#include "../../Components/GuiComponents/GuiComponent.h"
#include "../../Components/RigidbodyComponents/VehicleComponent.h"
#include "../../Systems/Content/ContentManager.h"
#include "../../Systems/Game.h"

SpeedPowerUp::SpeedPowerUp() { }

void SpeedPowerUp::Collect(PlayerData* player) {
    PowerUp::Collect(player);

    VehicleComponent* vehicle = player->vehicleEntity->GetComponent<VehicleComponent>();
    vehicle->speedMultiplier += multiplier;
    
    TweenVignette();
}

void SpeedPowerUp::RemoveInternal() {
    VehicleComponent* vehicle = player->vehicleEntity->GetComponent<VehicleComponent>();
    vehicle->speedMultiplier -= multiplier;
}

std::string SpeedPowerUp::GetGuiName() const {
    return "SpeedPowerUp";
}
