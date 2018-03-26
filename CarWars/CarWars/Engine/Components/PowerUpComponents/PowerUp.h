#pragma once

#include "../../Systems/Time.h"

class GuiComponent;
struct PlayerData;

class PowerUp {
public:
    explicit PowerUp(Time a_duration = 10.0);

    virtual void Collect(PlayerData* player);

    void TweenVignette() const;

    void Remove(bool force);
protected:
    virtual void RemoveInternal() = 0;
    virtual std::string GetGuiName() const = 0;

    float multiplier;

    PlayerData* player;
    
    Time collectedTime;
    Time duration;
};
