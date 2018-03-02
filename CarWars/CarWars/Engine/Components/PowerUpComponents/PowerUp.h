#pragma once

#include "../Component.h"
//#include "../Engine/Systems/Time.h"

class PowerUp : public Component {
    friend class SpeedPowerUp;
    friend class DefencePowerUp;
    friend class DamagePowerUp;
public:
    virtual void Collect() = 0;

    ComponentType GetType() override;
    void HandleEvent(Event *event) override;

    void RenderDebugGui() override;

private:
    float multiplier;
};