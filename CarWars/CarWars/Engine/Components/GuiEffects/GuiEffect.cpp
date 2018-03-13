#include "GuiEffect.h"
#include "../../Systems/StateManager.h"

GuiEffect::GuiEffect(Time _duration) : duration(_duration), startTime(StateManager::globalTime) {
    UpdateStartTime();
}

void GuiEffect::UpdateStartTime() {
    expireTime = StateManager::globalTime + duration;
}

void GuiEffect::AddDuration(Time _duration) {
    duration += _duration;
    expireTime += _duration;
}

bool GuiEffect::IsExpired() const {
    return StateManager::globalTime >= expireTime;
}

void GuiEffect::Update(GuiComponent* gui) {}
