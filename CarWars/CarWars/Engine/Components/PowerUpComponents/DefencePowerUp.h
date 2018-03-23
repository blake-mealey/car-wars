#pragma once

#include "PowerUp.h"

class DefencePowerUp : public PowerUp {
public:
    DefencePowerUp();
    void Collect(Entity* car) override;
    void SetEntity(Entity* _entity) override;

    ComponentType GetType() override;

    void RenderDebugGui() override;
private:
    float multiplier = 0.25f;
};