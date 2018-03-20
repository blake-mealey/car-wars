#pragma once
#include "SpeedPowerUp.h"

SpeedPowerUp::SpeedPowerUp() {}

void SpeedPowerUp::Collect() {
    std::cout << "Speed Collected" << std::endl;
}

void SpeedPowerUp::Collect(Entity* car) {
    //PowerUp::Collect();
    //PlayerData* player = Game::Instance().GetPlayerFromEntity(car);
    //if (player) {
    //    Entity* guiRoot = player->camera->GetGuiRoot();
    //    Entity* guiEntity = EntityManager::FindFirstChild(guiRoot, "DamagePowerUp");
    //    GuiComponent* gui = guiEntity->GetComponent<GuiComponent>();
    //    auto tween = Effects::Instance().CreateTween<float, easing::Quint::easeOut>(0.f, 1.f, 0.25);
    //    tween->Start();
    //}
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