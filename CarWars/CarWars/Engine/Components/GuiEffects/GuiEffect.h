#pragma once

#include "../../Systems/Time.h"

class GuiComponent;

class GuiEffect {
public:
    virtual ~GuiEffect() = default;
    GuiEffect(Time _duration = 0.5);

    void UpdateStartTime();
    void AddDuration(Time _duration);
    bool IsExpired() const;

    virtual void Apply(GuiComponent* gui) = 0;
    virtual void Remove(GuiComponent* gui) = 0;
    virtual void Update(GuiComponent* gui);
protected:
    Time startTime;
    Time expireTime;
    Time duration;
};
