#include "TransparencyEffect.h"
#include "../GuiComponents/GuiComponent.h"

TransparencyEffect::TransparencyEffect(Time duration, float _transparencyMod, Mode _mode) :
    GuiEffect(duration), transparencyMod(_transparencyMod), mode(_mode) { }

void TransparencyEffect::Apply(GuiComponent* gui) {
    switch (mode) {
    case Add:
        gui->SetTextureColor(gui->GetTextureColor() + glm::vec4(0.f, 0.f, 0.f, transparencyMod));
        break;
    case Multiply:
        gui->SetTextureColor(gui->GetTextureColor() * glm::vec4(1.f, 1.f, 1.f, transparencyMod));
        break;
    case Set:
        previousTrans = gui->GetTextureColor().a;
        gui->SetTextureColor(gui->GetTextureColor() * glm::vec4(1.f, 1.f, 1.f, 0.f) + glm::vec4(0.f, 0.f, 0.f, transparencyMod));
        break;
    }
}

void TransparencyEffect::Remove(GuiComponent* gui) {
    switch (mode) {
    case Add:
        gui->SetTextureColor(gui->GetTextureColor() - glm::vec4(0.f, 0.f, 0.f, transparencyMod));
        break;
    case Multiply:
        gui->SetTextureColor(gui->GetTextureColor() / glm::vec4(1.f, 1.f, 1.f, transparencyMod));
        break;
    case Set:
        gui->SetTextureColor(gui->GetTextureColor() * glm::vec4(1.f, 1.f, 1.f, 0.f) + glm::vec4(0.f, 0.f, 0.f, previousTrans));
        break;
    }
}
