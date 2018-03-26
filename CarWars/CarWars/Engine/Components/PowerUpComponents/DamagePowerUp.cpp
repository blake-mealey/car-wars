#pragma once

#include "DamagePowerUp.h"
#include "../../Components/GuiComponents/GuiComponent.h"
#include "../../Components/WeaponComponents/WeaponComponent.h"
#include "../../Systems/Game.h"
#include "../../Systems/Content/ContentManager.h"

DamagePowerUp::DamagePowerUp() { }

void DamagePowerUp::Collect(PlayerData* player) {
    PowerUp::Collect(player);

    WeaponComponent* weapon = player->vehicleEntity->GetComponent<WeaponComponent>();
    weapon->damageMultiplier = 1.25f;

    TweenVignette();
}

void DamagePowerUp::RemoveInternal() {
    WeaponComponent* weapon = player->vehicleEntity->GetComponent<WeaponComponent>();
    weapon->damageMultiplier = 1.f;
}

std::string DamagePowerUp::GetGuiName() const {
    return "DamagePowerUp";
}

glm::vec4 DamagePowerUp::GetColor() const {
	return glm::vec4(1.f, 0.f, 0.f, 1.f);
}
