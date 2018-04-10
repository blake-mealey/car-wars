#pragma once
#include "PowerUp.h"
#include "../../Components/GuiComponents/GuiComponent.h"
#include "../../Components/RigidbodyComponents/VehicleComponent.h"
#include "../../Systems/Content/ContentManager.h"
#include "../../Systems/Game.h"
#include "../../Systems/StateManager.h"

class HealthPowerUp : public PowerUp {
public:
    HealthPowerUp();

    void Collect(PlayerData* player) override;
    void RemoveInternal() override;
    void Tick(PlayerData* player);

    glm::vec4 GetColor() const;
protected:
    std::string GetGuiName() const override;
private:
    float collectedAt;
    float health = 250.f;
    int numTicks = 4;
};