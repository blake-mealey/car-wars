#pragma once

#include "PowerUp.h"

class SpeedPowerUp : public PowerUp {
public:
    SpeedPowerUp();
    
    void Collect(PlayerData* player) override;
    void RemoveInternal() override;
private:
    float multiplier = 10.f;
};