#pragma once

#include "GuiEffect.h"

class TransparencyEffect : public GuiEffect {
public:
    enum Mode { Add = 0, Multiply, Set };

    ~TransparencyEffect() = default;
    TransparencyEffect(Time duration, float _transparencyMod, Mode _mode = Set);

    void Apply(GuiComponent* gui) override;
    void Remove(GuiComponent* gui) override;
private:
    float transparencyMod;
    float previousTrans;
    Mode mode;
};

