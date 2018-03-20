#pragma once
#include "DefencePowerUp.h"

DefencePowerUp::DefencePowerUp() {}

void DefencePowerUp::Collect() {
    std::cout << "Defence Collected" << std::endl;
}

void DefencePowerUp::Collect(Entity* car) {
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

ComponentType DefencePowerUp::GetType() {
    return ComponentType_DefencePowerUp;
}

void DefencePowerUp::HandleEvent(Event *event) {
    return;
}

void DefencePowerUp::RenderDebugGui() {
    PowerUp::RenderDebugGui();
}