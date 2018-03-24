#pragma once

#include "DefencePowerUp.h"
#include "../../Components/GuiComponents/GuiComponent.h"
#include "../../Components/RigidbodyComponents/VehicleComponent.h"
#include "../../Systems/Content/ContentManager.h"
#include "../../Systems/Game.h"

DefencePowerUp::DefencePowerUp() { }

void DefencePowerUp::Collect(PlayerData* player) {
    PowerUp::Collect(player);

    VehicleComponent* vehicle = player->vehicleEntity->GetComponent<VehicleComponent>();
    vehicle->defenceMultiplier = 1.25f;
    
    TweenVignette("DefencePowerUp");
}

void DefencePowerUp::RemoveInternal() {
    VehicleComponent* vehicle = player->vehicleEntity->GetComponent<VehicleComponent>();
    vehicle->defenceMultiplier = 1.f;
}
