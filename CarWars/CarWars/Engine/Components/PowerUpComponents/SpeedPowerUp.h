#pragma once

#include "PowerUp.h"

class SpeedPowerUp : public PowerUp {
public:
    SpeedPowerUp();
    
    void Collect(PlayerData* player) override;
    void RemoveInternal() override;

	glm::vec4 GetColor() const;
protected:
    std::string GetGuiName() const override;
    float multiplier = 10.f;
};