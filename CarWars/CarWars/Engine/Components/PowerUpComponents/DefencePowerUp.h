#pragma once
#include <iostream>

#include "../../Systems/StateManager.h"

#include "PowerUp.h"

class DefencePowerUp : public PowerUp {
public:
    DefencePowerUp();
    void Collect() override;
    void Collect(Entity* car) override;

    ComponentType GetType() override;
    void HandleEvent(Event *event) override;

    void RenderDebugGui() override;
private:
    float multiplier = 0.25f;
};