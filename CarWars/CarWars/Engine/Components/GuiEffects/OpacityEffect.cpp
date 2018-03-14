#include "OpacityEffect.h"
#include "../GuiComponents/GuiComponent.h"
#include "../../Systems/StateManager.h"
#include "PennerEasing/Quint.h"

OpacityEffect::OpacityEffect(Time duration, float _opacityMod, Time _tweenInTime, Time _tweenOutTime) :
    GuiEffect(duration), opacityMod(_opacityMod), tweenInTime(_tweenInTime), tweenOutTime(_tweenOutTime) { }

void OpacityEffect::Apply(GuiComponent* gui) {
    previousTextureOpacity = gui->GetTextureOpacity();
    previousFontOpacity = gui->GetFontOpacity();

    // Check for no tween?
}

void OpacityEffect::Remove(GuiComponent* gui) {
    gui->SetTextureOpacity(previousTextureOpacity);
    gui->SetFontOpacity(previousFontOpacity);
}

void OpacityEffect::Update(GuiComponent* gui) {
    const Time time = StateManager::globalTime;
    
    float thisTextureOpacity = opacityMod;
    float thisFontOpacity = opacityMod;

    // tweening in
    if (time <= startTime + tweenInTime) {
        thisTextureOpacity = easing::Quint::easeOut(
            (time - startTime).GetSeconds(),
            previousTextureOpacity,
            opacityMod - previousTextureOpacity,
            tweenInTime.GetSeconds());
        thisFontOpacity = easing::Quint::easeOut(
            (time - startTime).GetSeconds(),
            previousFontOpacity,
            opacityMod - previousFontOpacity,
            tweenInTime.GetSeconds());
    }

    // tweening out
    if (time >= expireTime - tweenOutTime) {
        thisTextureOpacity = easing::Quint::easeIn(
            (time - (expireTime - tweenOutTime)).GetSeconds(),
            opacityMod,
            previousTextureOpacity - opacityMod,
            tweenOutTime.GetSeconds());
        thisFontOpacity = easing::Quint::easeIn(
            (time - (expireTime - tweenOutTime)).GetSeconds(),
            opacityMod,
            previousFontOpacity - opacityMod,
            tweenOutTime.GetSeconds());
    }
    
    gui->SetTextureOpacity(thisTextureOpacity);
    gui->SetFontOpacity(thisFontOpacity);
}
