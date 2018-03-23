#pragma once

#include "../../Systems/Time.h"

struct PlayerData;

class PowerUp {
public:
    PowerUp(Time a_duration = 5.0);

    virtual void Collect(PlayerData* player);
private:
    float multiplier;
    
    Time collectedTime;
    Time duration;
};
