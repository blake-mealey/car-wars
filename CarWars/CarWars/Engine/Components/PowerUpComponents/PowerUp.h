#pragma once

#include "../../Systems/Time.h"
#include "glm/glm.hpp"

class GuiComponent;
struct PlayerData;

class PowerUp {
public:
    explicit PowerUp(Time a_duration = 10.0);

    virtual void Collect(PlayerData* player);

    void TweenVignette() const;

    void Remove(bool force);

	virtual glm::vec4 GetColor() const = 0;
protected:
    virtual void RemoveInternal() = 0;
    virtual std::string GetGuiName() const = 0;

    float multiplier;

    PlayerData* player;
    
    Time collectedTime;
    Time duration;
};
