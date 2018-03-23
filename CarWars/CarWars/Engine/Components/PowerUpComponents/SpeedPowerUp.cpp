#pragma once

#include "SpeedPowerUp.h"
#include "../../Components/GuiComponents/GuiComponent.h"
#include "../../Components/RigidbodyComponents/VehicleComponent.h"
#include "../../Systems/Content/ContentManager.h"
#include "../../Systems/Game.h"

SpeedPowerUp::SpeedPowerUp() { }

void SpeedPowerUp::Collect(PlayerData* player) {
    std::cout << "Speed Collected" << std::endl;

    PowerUp::Collect(player);

    VehicleComponent* vehicle = player->vehicleEntity->GetComponent<VehicleComponent>();
    vehicle->speedMultiplier += multiplier;
    
    TweenVignette("SpeedPowerUp");
}

void SpeedPowerUp::RemoveInternal() {
    VehicleComponent* vehicle = player->vehicleEntity->GetComponent<VehicleComponent>();
    vehicle->speedMultiplier -= multiplier;
}
