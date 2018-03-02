#pragma once
#include "DamagePowerUp.h"

DamagePowerUp::DamagePowerUp() {}

void DamagePowerUp::Collect() {
    std::cout << "Damage Collected" << std::endl;
}

ComponentType DamagePowerUp::GetType() {
    return ComponentType_DamagePowerUp;
}

void DamagePowerUp::HandleEvent(Event *event) {
    return;
}

void DamagePowerUp::RenderDebugGui() {
    PowerUp::RenderDebugGui();
}