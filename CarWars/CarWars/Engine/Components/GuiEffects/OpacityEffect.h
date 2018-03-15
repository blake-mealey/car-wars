#pragma once

#include "GuiEffect.h"

class OpacityEffect : public GuiEffect {
public:
    ~OpacityEffect() = default;
    OpacityEffect(Time duration, float _opacityMod, Time _tweenInTime = 0.0, Time _tweenOutTime = 0.0);

    void Apply(GuiComponent* gui) override;
    void Remove(GuiComponent* gui) override;
    void Update(GuiComponent* gui) override;
private:
    float finish;
    float opacityMod;
    float previousTextureOpacity;
    float previousFontOpacity;

    bool startedTweenIn;
    bool startedTweenOut;

    Time tweenInTime;
    Time tweenOutTime;
};

