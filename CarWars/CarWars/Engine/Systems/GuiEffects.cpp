#include "GuiEffects.h"

#include "../Components/GuiEffects/GuiEffect.h"
#include "../Components/Component.h"
#include "../Components/GuiComponents/GuiComponent.h"
#include "../Entities/EntityManager.h"

// Singleton
GuiEffects::GuiEffects() {}
GuiEffects &GuiEffects::Instance() {
    static GuiEffects instance;
    return instance;
}

GuiEffects::~GuiEffects() {
    std::vector<Component*> guis = EntityManager::GetComponents(ComponentType_GUI);
    for (Component* component : guis) {
        GuiComponent* gui = static_cast<GuiComponent*>(component);
        for (GuiEffect* effect : gui->GetEffects()) {
            gui->RemoveEffect(effect);
            delete effect;
        }
    }
}

void GuiEffects::Update() {
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
}
