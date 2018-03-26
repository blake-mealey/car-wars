#pragma once

#include "PowerUp.h"

class DefencePowerUp : public PowerUp {
public:
    DefencePowerUp();

    void Collect(PlayerData* player) override;
    void RemoveInternal() override;

	glm::vec4 GetColor() const;
protected:
    std::string GetGuiName() const override;
    float multiplier = 0.25f;
};