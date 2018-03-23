#pragma once

#include "DamagePowerUp.h"
#include "../../Components/GuiComponents/GuiComponent.h"
#include "../../Components/WeaponComponents/WeaponComponent.h"
#include "../../Systems/Game.h"
#include "../../Systems/Content/ContentManager.h"

DamagePowerUp::DamagePowerUp() { }

void DamagePowerUp::Collect(PlayerData* player) {
    std::cout << "Damage Collected" << std::endl;

    PowerUp::Collect(player);

    WeaponComponent* weapon = player->vehicleEntity->GetComponent<WeaponComponent>();
    weapon->damageMultiplier = 1.25f;

    TweenVignette("DamagePowerUp");
}

void DamagePowerUp::RemoveInternal() {
    WeaponComponent* weapon = player->vehicleEntity->GetComponent<WeaponComponent>();
    weapon->damageMultiplier = 1.f;
}
