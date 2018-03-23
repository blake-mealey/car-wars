#pragma once

#include "PowerUp.h"

class SpeedPowerUp : public PowerUp {
public:
    SpeedPowerUp();
    void Collect(Entity* car) override;
    void SetEntity(Entity* _entity) override;

    ComponentType GetType() override;

    void RenderDebugGui() override;
private:
    float multiplier = 0.15f;
};