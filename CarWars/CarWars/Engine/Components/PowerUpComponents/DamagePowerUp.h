#pragma once

#include "PowerUp.h"

class DamagePowerUp : public PowerUp {
public:
    DamagePowerUp();

    void Collect(PlayerData* player) override;
    void RemoveInternal() override;
protected:
    std::string GetGuiName() const override;
    float multiplier = 0.25f;
};