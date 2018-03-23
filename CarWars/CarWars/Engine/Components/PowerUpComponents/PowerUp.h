#pragma once

#include "../../Systems/Time.h"

class GuiComponent;
struct PlayerData;

class PowerUp {
public:
    explicit PowerUp(Time a_duration = 10.0);

    virtual void Collect(PlayerData* player);

    void TweenVignette(std::string guiName) const;

    void Remove();
protected:
    virtual void RemoveInternal() = 0;

    float multiplier;

    PlayerData* player;
    
    Time collectedTime;
    Time duration;
};
