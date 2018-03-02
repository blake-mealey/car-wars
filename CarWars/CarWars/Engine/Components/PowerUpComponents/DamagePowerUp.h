#pragma once
#include <iostream>

#include "../../Systems/StateManager.h"

#include "PowerUp.h"

class DamagePowerUp : public PowerUp {
public:
    DamagePowerUp();
    void Collect() override;

    ComponentType GetType() override;
    void HandleEvent(Event *event) override;

    void RenderDebugGui() override;
private:
    float multiplier = 0.25f;
};