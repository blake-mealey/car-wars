#pragma once

#include "System.h"

class GuiEffect;

class GuiEffects : public System {
public:
    // Access the singleton instance
    static GuiEffects& Instance();
    ~GuiEffects();

    void Update() override;

private:
    // No instantiation or copying
    GuiEffects();
    GuiEffects(const GuiEffects&) = delete;
    GuiEffects& operator= (const GuiEffects&) = delete;
};
