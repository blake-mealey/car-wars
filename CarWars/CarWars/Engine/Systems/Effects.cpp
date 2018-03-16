#include "Effects.h"

#include "../Components/GuiEffects/GuiEffect.h"
#include "../Components/Component.h"
#include "../Components/GuiComponents/GuiComponent.h"
#include "../Entities/EntityManager.h"

// Singleton
Effects::Effects() {}
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

void Effects::Update() {
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
}
