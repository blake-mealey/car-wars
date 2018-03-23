#pragma once

#include "PowerUp.h"

class DamagePowerUp : public PowerUp {
public:
    DamagePowerUp();
    void Collect(Entity* car) override;

    void SetEntity(Entity* _entity) override;

    ComponentType GetType() override;

    void RenderDebugGui() override;
private:
    float multiplier = 0.25f;
};