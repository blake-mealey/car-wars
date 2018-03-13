#pragma once

#include "GuiEffect.h"

class OpacityEffect : public GuiEffect {
public:
    enum Mode { Add = 0, Multiply, Set };

    ~OpacityEffect() = default;
    OpacityEffect(Time duration, float _opacityMod, Mode _mode = Set);

    void Apply(GuiComponent* gui) override;
    void Remove(GuiComponent* gui) override;
private:
    float opacityMod;
    float previousTextureOpacity;
    float previousFontOpacity;
    Mode mode;
};

