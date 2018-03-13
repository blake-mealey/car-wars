#include "GuiEffect.h"
#include "../../Systems/StateManager.h"

GuiEffect::GuiEffect(Time duration) {
    expireTime = StateManager::globalTime + duration;
}

void GuiEffect::UpdateDuration(Time duration) {
    expireTime = StateManager::globalTime + duration;
}

void GuiEffect::AddDuration(Time duration) {
    expireTime += duration;
}

bool GuiEffect::IsExpired() const {
    return StateManager::globalTime >= expireTime;
}

