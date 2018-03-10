#include "GuiHelper.h"

#include "../Entities/Entity.h"
#include "../Components/Component.h"
#include "../Components/GuiComponents/GuiComponent.h"
#include "../Entities/EntityManager.h"
#include "Content/ContentManager.h"

GuiComponent* GuiHelper::GetSelectedGui(Entity* entity) {
    for (Component *component : entity->GetComponents<GuiComponent>()) {
        GuiComponent *gui = static_cast<GuiComponent*>(component);
        if (gui->IsSelected()) return gui;
    }
}

GuiComponent* GuiHelper::GetSelectedGui(std::string entityTag, int playerIndex) {
    return GetSelectedGui(EntityManager::FindEntities(entityTag)[playerIndex]);
}

std::string GuiHelper::GetSelectedGuiText(Entity* entity) {
    return GetSelectedGui(entity)->GetText();
}

std::string GuiHelper::GetSelectedGuiText(std::string entityTag, int playerIndex) {
    return GetSelectedGui(entityTag, playerIndex)->GetText();
}

void GuiHelper::SelectNextGui(Entity* entity) {
	std::vector<GuiComponent*> components = entity->GetComponents<GuiComponent>();
    for (auto it = components.begin(); it != components.end(); ++it) {
        GuiComponent *gui = *it;
        if (gui->IsSelected()) {
            gui->SetSelected(false);
            if (it + 1 != components.end()) {
                static_cast<GuiComponent*>(*(it + 1))->SetSelected(true);
            } else {
                static_cast<GuiComponent*>(components.front())->SetSelected(true);
            }
            break;
        }
    }
}

void GuiHelper::SelectNextGui(std::string entityTag, int playerIndex) {
    return SelectNextGui(EntityManager::FindEntities(entityTag)[playerIndex]);
}

void GuiHelper::SelectPreviousGui(Entity* entity) {
	std::vector<GuiComponent*> components = entity->GetComponents<GuiComponent>();
    for (auto it = components.begin(); it != components.end(); ++it) {
		GuiComponent *gui = *it;
        if (gui->IsSelected()) {
            gui->SetSelected(false);
            if (it - 1 != components.begin() - 1) {
                static_cast<GuiComponent*>(*(it - 1))->SetSelected(true);
            } else {
                static_cast<GuiComponent*>(components.back())->SetSelected(true);
            }
            break;
        }
    }
}

void GuiHelper::SelectPreviousGui(std::string entityTag, int playerIndex) {
    return SelectPreviousGui(EntityManager::FindEntities(entityTag)[playerIndex]);
}

void GuiHelper::LoadGuiSceneToCamera(size_t cameraIndex, std::string guiScene) {
    CameraComponent *camera = static_cast<CameraComponent*>(EntityManager::GetComponents(ComponentType_Camera)[cameraIndex]);
    ContentManager::LoadScene(guiScene, camera->GetGuiRoot());
}

void GuiHelper::SetGuisEnabled(Entity *entity, bool enabled) {
	for (Component *component : entity->GetComponents<GuiComponent>()) {
		component->enabled = enabled;
	}
}

void GuiHelper::SetGuisEnabled(std::string entityTag, bool enabled, int playerIndex) {
	SetGuisEnabled(EntityManager::FindEntities(entityTag)[playerIndex], enabled);
}

void GuiHelper::DestroyGuis(Entity* entity) {
	std::vector<GuiComponent*> guis = entity->GetComponents<GuiComponent>();
	for (GuiComponent* gui : guis) {
		EntityManager::DestroyComponent(gui);
	}
}

void GuiHelper::DestroyGuis(std::string entityTag, int playerIndex) {
	DestroyGuis(EntityManager::FindEntities(entityTag)[playerIndex]);
}

void GuiHelper::SetGuiText(Entity* entity, int guiIndex, std::string text) {
    std::vector<GuiComponent*> guis = entity->GetComponents<GuiComponent>();
    if (guis.size() <= guiIndex) return;
    guis[guiIndex]->SetText(text);
}

void GuiHelper::SetGuiText(std::string entityTag, int guiIndex, std::string text, int playerIndex) {
    SetGuiText(EntityManager::FindEntities(entityTag)[playerIndex], guiIndex, text);
}

void GuiHelper::SetFirstGuiText(Entity *entity, std::string text) {
    SetGuiText(entity, 0, text);
}

void GuiHelper::SetFirstGuiText(std::string entityTag, std::string text, int playerIndex) {
	SetFirstGuiText(EntityManager::FindEntities(entityTag)[playerIndex], text);
}

void GuiHelper::SetSecondGuiText(Entity* entity, std::string text) {
    SetGuiText(entity, 1, text);
}

void GuiHelper::SetSecondGuiText(std::string entityTag, std::string text, int playerIndex) {
    SetSecondGuiText(EntityManager::FindEntities(entityTag)[playerIndex], text);
}

bool GuiHelper::FirstGuiHasText(Entity *entity, std::string text) {
	GuiComponent *gui = entity->GetComponent<GuiComponent>();
    if (!gui) return false;
    return gui->HasText(text);
}

bool GuiHelper::FirstGuiHasText(std::string entityTag, std::string text, int playerIndex) {
	return FirstGuiHasText(EntityManager::FindEntities(entityTag)[playerIndex], text);
}

bool GuiHelper::FirstGuiContainsText(Entity* entity, std::string text) {
    GuiComponent *gui = entity->GetComponent<GuiComponent>();
    if (!gui) return false;
    return gui->ContainsText(text);
}

bool GuiHelper::FirstGuiContainsText(std::string entityTag, std::string text, int playerIndex) {
    return FirstGuiContainsText(EntityManager::FindEntities(entityTag)[playerIndex], text);
}

void GuiHelper::SetGuiPositions(Entity* entity, glm::vec3 position) {
    for (GuiComponent* gui : entity->GetComponents<GuiComponent>()) {
        gui->transform.SetPosition(position);
    }
}

void GuiHelper::SetGuiPositions(std::string entityTag, glm::vec3 position, int playerIndex) {
    SetGuiPositions(EntityManager::FindEntities(entityTag)[playerIndex], position);
}
