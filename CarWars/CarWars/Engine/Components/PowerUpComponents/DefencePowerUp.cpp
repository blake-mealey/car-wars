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
    
    TweenVignette();
}

void DefencePowerUp::RemoveInternal() {
    VehicleComponent* vehicle = player->vehicleEntity->GetComponent<VehicleComponent>();
    vehicle->defenceMultiplier = 1.f;
}

std::string DefencePowerUp::GetGuiName() const {
    return "DefencePowerUp";
}

glm::vec4 DefencePowerUp::GetColor() const {
	return glm::vec4(0.f, 1.f, 1.f, 1.f);
}
