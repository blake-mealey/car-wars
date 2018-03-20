#pragma once
#include <iostream>

#include "../../Systems/StateManager.h"

#include "PowerUp.h"

class SpeedPowerUp : public PowerUp {
public:
    SpeedPowerUp();
    void Collect(Entity* car) override;
    void Collect() override;

    ComponentType GetType() override;
    void HandleEvent(Event *event) override;

    void RenderDebugGui() override;
private:
    float multiplier = 0.15f;
};