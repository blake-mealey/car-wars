#pragma once
#include "PowerUp.h"

void PowerUp::Collect() {
    std::cout<<"collect was called"<<std::endl;
    Audio& audioManager = Audio::Instance();
    audioManager.PlayAudio("Content/Sounds/powerup.mp3");
}

void PowerUp::Collect(Entity* car) {
    PowerUp::Collect();
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