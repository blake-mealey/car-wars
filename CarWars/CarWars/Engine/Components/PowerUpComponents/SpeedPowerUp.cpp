#pragma once
#include "SpeedPowerUp.h"

SpeedPowerUp::SpeedPowerUp() {}

void SpeedPowerUp::Collect() {
    std::cout << "Speed Collected" << std::endl;
}

ComponentType SpeedPowerUp::GetType() {
    return ComponentType_SpeedPowerUp;
}

void SpeedPowerUp::HandleEvent(Event *event) {
    return;
}

void SpeedPowerUp::RenderDebugGui() {
    PowerUp::RenderDebugGui();
}