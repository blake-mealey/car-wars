#pragma once

#include "../Component.h"

class PowerUp : public Component {
public:
    virtual void Collect(Entity *car) = 0;

    ComponentType GetType() override;
    void HandleEvent(Event *event) override;

    void RenderDebugGui() override;

private:
    float multiplier;
};