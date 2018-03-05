#pragma once
#include "PowerUp.h"

void PowerUp::Collect() {
    Audio& audioManager = Audio::Instance();
    audioManager.PlayAudio("Content/Sounds/powerup.mp3");
}

ComponentType PowerUp::GetType() {
    return ComponentType_PowerUp;
}

void PowerUp::HandleEvent(Event *event) {
    return;
}

void PowerUp::RenderDebugGui() {
    Component::RenderDebugGui();
}