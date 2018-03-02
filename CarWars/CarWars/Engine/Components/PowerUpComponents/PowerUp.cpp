#pragma once
#include "PowerUp.h"

ComponentType PowerUp::GetType() {
    return ComponentType_PowerUp;
}

void PowerUp::HandleEvent(Event *event) {
    return;
}

void PowerUp::RenderDebugGui() {
    Component::RenderDebugGui();
}