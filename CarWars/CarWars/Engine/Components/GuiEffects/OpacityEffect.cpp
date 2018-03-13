#include "OpacityEffect.h"
#include "../GuiComponents/GuiComponent.h"

OpacityEffect::OpacityEffect(Time duration, float _opacityMod, Mode _mode) :
    GuiEffect(duration), opacityMod(_opacityMod), mode(_mode) { }

void OpacityEffect::Apply(GuiComponent* gui) {
    switch (mode) {
    case Add:
        gui->AddOpacity(opacityMod);
        break;
    case Multiply:
        gui->MultiplyOpacity(opacityMod);
        break;
    case Set:
        previousTextureOpacity = gui->GetTextureOpacity();
        previousFontOpacity = gui->GetFontOpacity();
        gui->SetOpacity(opacityMod);
        break;
    }
}

void OpacityEffect::Remove(GuiComponent* gui) {
    switch (mode) {
    case Add:
        gui->AddOpacity(-opacityMod);
        break;
    case Multiply:
        gui->MultiplyOpacity(1.f / opacityMod);
        break;
    case Set:
        gui->SetTextureOpacity(previousTextureOpacity);
        gui->SetFontOpacity(previousFontOpacity);
        break;
    }
}
