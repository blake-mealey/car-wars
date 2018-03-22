#include "Effects.h"

#include "../Components/GuiEffects/GuiEffect.h"
#include "../Components/Component.h"
#include "../Components/GuiComponents/GuiComponent.h"
#include "../Entities/EntityManager.h"

// Singleton
Effects::Effects() : inUpdate(false) {}
Effects &Effects::Instance() {
    static Effects instance;
    return instance;
}

Effects::~Effects() {
    std::vector<Component*> guis = EntityManager::GetComponents(ComponentType_GUI);
    for (Component* component : guis) {
        GuiComponent* gui = static_cast<GuiComponent*>(component);
        for (GuiEffect* effect : gui->GetEffects()) {
            gui->RemoveEffect(effect);
            delete effect;
        }
    }
}

void Effects::DestroyTween(Tween* tween) {
    const auto it = std::find(tweens.begin(), tweens.end(), tween);
    if (it != tweens.end()) tweens.erase(it);
    tween->Stop(false);
    delete tween;
}

void Effects::DestroyTweens() {
    while (!tweens.empty()) {
        DestroyTween(tweens.back());
    }
}

Tween* Effects::FindTween(std::string tag) {
    for (Tween* tween : tweens) {
        if (tween->HasTag(tag)) return tween;
    }
    return nullptr;
}

void Effects::Update() {
    inUpdate = true;

    std::vector<Component*> guis = EntityManager::GetComponents(ComponentType_GUI);
    for (Component* component : guis) {
        GuiComponent* gui = static_cast<GuiComponent*>(component);
        for (GuiEffect* effect : gui->GetEffects()) {
            if (effect->IsExpired()) {
                gui->RemoveEffect(effect);
                delete effect;
            } else {
                effect->Update(gui);
            }
        }
    }

    for (auto it = tweens.begin(); it != tweens.end();) {
        Tween* tween = *it;
        tween->Update();
        if (tween->IsOwnedByEffectsSystem() && tween->Finished()) {
            it = tweens.erase(it);
            delete tween;
        } else {
            ++it;
        }
    }

    if (!tweensCreatedInUpdate.empty()) {
        for (Tween* tween : tweensCreatedInUpdate) {
            tweens.push_back(tween);
        }
        tweensCreatedInUpdate.clear();
    }

    inUpdate = false;
}
