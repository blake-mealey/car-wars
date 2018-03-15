#include "OpacityEffect.h"
#include "../GuiComponents/GuiComponent.h"
#include "../../Systems/StateManager.h"
#include "PennerEasing/Quint.h"
#include "../../Systems/Effects.h"

OpacityEffect::OpacityEffect(Time duration, float _opacityMod, Time _tweenInTime, Time _tweenOutTime) :
    GuiEffect(duration), opacityMod(_opacityMod), startedTweenIn(false), startedTweenOut(false), tweenInTime(_tweenInTime), tweenOutTime(_tweenOutTime) { }

void OpacityEffect::Apply(GuiComponent* gui) {
    previousTextureOpacity = gui->GetTextureOpacity();
    previousFontOpacity = gui->GetFontOpacity();
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
    if (!startedTweenIn && time <= startTime + tweenInTime) {
        startedTweenIn = true;
        Effects::Instance().CreateTween<float, easing::Quint::easeOut>(thisTextureOpacity, previousTextureOpacity, opacityMod, tweenInTime.GetSeconds())->Start();
        Effects::Instance().CreateTween<float, easing::Quint::easeOut>(thisFontOpacity, previousFontOpacity, opacityMod, tweenInTime.GetSeconds())->Start();
    }

    // tweening out
    if (!startedTweenOut && time >= expireTime - tweenOutTime) {
        startedTweenOut = true;
        Effects::Instance().CreateTween<float, easing::Quint::easeIn>(thisTextureOpacity, opacityMod, previousTextureOpacity, tweenOutTime.GetSeconds())->Start();
        Effects::Instance().CreateTween<float, easing::Quint::easeIn>(thisFontOpacity, opacityMod, previousFontOpacity, tweenOutTime.GetSeconds())->Start();
    }
    
    gui->SetTextureOpacity(thisTextureOpacity);
    gui->SetFontOpacity(thisFontOpacity);
}
