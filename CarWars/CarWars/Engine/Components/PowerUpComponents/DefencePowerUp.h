#pragma once

#include "PowerUp.h"

class DefencePowerUp : public PowerUp {
public:
    DefencePowerUp();

    void Collect(PlayerData* player) override;
    void RemoveInternal() override;
private:
    float multiplier = 0.25f;
};