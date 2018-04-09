#pragma once
#include "PowerUp.h"

class HealthPowerUp : public PowerUp {
public:
    HealthPowerUp();

    void Collect(PlayerData* player) override;
    void RemoveInternal() override;

    glm::vec4 GetColor() const;
protected:
    std::string GetGuiName() const override;
};