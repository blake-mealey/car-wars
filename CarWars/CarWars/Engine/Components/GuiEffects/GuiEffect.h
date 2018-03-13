#pragma once

#include "../../Systems/Time.h"

class GuiComponent;

class GuiEffect {
public:
    virtual ~GuiEffect() = default;
    GuiEffect(Time duration = 0.5);

    void UpdateDuration(Time duration);
    void AddDuration(Time duration);
    bool IsExpired() const;

    virtual void Apply(GuiComponent* gui) = 0;
    virtual void Remove(GuiComponent* gui) = 0;
    virtual void Update(GuiComponent* gui) const {};
private:
    Time expireTime;
};
