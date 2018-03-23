#pragma once

#include "PowerUp.h"

class DamagePowerUp : public PowerUp {
public:
    DamagePowerUp();
    void Collect(PlayerData* player) override;
private:
    float multiplier = 0.25f;
};