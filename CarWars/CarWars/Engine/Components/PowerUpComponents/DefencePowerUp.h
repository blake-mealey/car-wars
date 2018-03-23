#pragma once

#include "PowerUp.h"

class DefencePowerUp : public PowerUp {
public:
    DefencePowerUp();
    void Collect(PlayerData* player) override;
private:
    float multiplier = 0.25f;
};