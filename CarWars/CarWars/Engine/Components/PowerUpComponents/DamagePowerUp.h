#pragma once

#include "PowerUp.h"
#include "glm/glm.hpp"

class DamagePowerUp : public PowerUp {
public:
    DamagePowerUp();

    void Collect(PlayerData* player) override;
    void RemoveInternal() override;

	glm::vec4 GetColor() const;
protected:
    std::string GetGuiName() const override;
    float multiplier = 0.25f;
};