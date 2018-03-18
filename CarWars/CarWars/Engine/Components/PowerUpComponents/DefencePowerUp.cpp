#pragma once
#include "DefencePowerUp.h"

DefencePowerUp::DefencePowerUp() {}

void DefencePowerUp::Collect() {
    std::cout << "Defence Collected" << std::endl;
}

ComponentType DefencePowerUp::GetType() {
    return ComponentType_DefencePowerUp;
}

void DefencePowerUp::HandleEvent(Event *event) {
    return;
}

void DefencePowerUp::RenderDebugGui() {
    PowerUp::RenderDebugGui();
}