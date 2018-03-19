#include "GuiHelper.h"

#include "../../Entities/EntityManager.h"
#include "../../Systems/Content/ContentManager.h"
#include "GuiComponent.h"

GuiComponent* GuiHelper::GetSelectedGui(Entity* entity) {
    for (GuiComponent* gui : entity->GetComponents<GuiComponent>()) {
        if (gui->IsSelected()) return gui;
    }
    return nullptr;
}

GuiComponent* GuiHelper::GetSelectedGui(std::string entityTag, int playerIndex) {
    return GetSelectedGui(EntityManager::FindEntities(entityTag)[playerIndex]);
}

Entity* GuiHelper::GetSelectedEntity(Entity* parent) {
    for (Entity* child : EntityManager::GetChildren(parent)) {
        if (IsEntitySelected(child)) return child;
    }
    return nullptr;
}

Entity* GuiHelper::GetSelectedEntity(std::string parentTag, int playerIndex) {
    return GetSelectedEntity(EntityManager::FindEntities(parentTag)[playerIndex]);
}

std::string GuiHelper::GetSelectedGuiText(Entity* entity) {
    return GetSelectedGui(entity)->GetText();
}

std::string GuiHelper::GetSelectedGuiText(std::string entityTag, int playerIndex) {
    return GetSelectedGui(entityTag, playerIndex)->GetText();
}

void GuiHelper::SelectNextGui(Entity* entity, int dir) {
    if (dir == 0) return;
    dir = dir == 0 ? 0 : dir < 0 ? -1 : 1;

    std::vector<GuiComponent*> components = entity->GetComponents<GuiComponent>();
    for (auto it = components.begin(); it != components.end(); ++it) {
        // Find the currently selected GUI and make it not selected
        GuiComponent *gui = *it;
        if (!gui->IsSelected()) continue;
        gui->SetSelected(false);

        // Try to find the next GUI in the given direction (wrapping) that is enabled
        // and not the same GUI and make it selected
        auto it2 = it;
        while ((it2 += dir) != it) {
            // Wrap
            if (it2 == components.end()) {
                it2 = components.begin();
            } else if (it2 < components.begin()) {
                it2 = components.end() - 1;
            }
            
            // Make sure it is enabled
            GuiComponent* gui2 = *it2;
            if (!gui2->enabled) continue;

            // Select it and finish
            gui2->SetSelected(true);
            break;
        }

        break;
    }
}

void GuiHelper::SelectNextGui(Entity* entity) {
    SelectNextGui(entity, 1);
}

void GuiHelper::SelectNextGui(std::string entityTag, int playerIndex) {
    SelectNextGui(EntityManager::FindEntities(entityTag)[playerIndex]);
}

void GuiHelper::SelectPreviousGui(Entity* entity) {
    SelectNextGui(entity, -1);
}

void GuiHelper::SelectPreviousGui(std::string entityTag, int playerIndex) {
    SelectPreviousGui(EntityManager::FindEntities(entityTag)[playerIndex]);
}

bool GuiHelper::IsEntitySelected(Entity* entity) {
    GuiComponent* gui = entity->GetComponent<GuiComponent>();
    return gui && gui->IsSelected();
}

bool GuiHelper::IsEntityEnabled(Entity* entity) {
    GuiComponent* gui = entity->GetComponent<GuiComponent>();
    return gui && gui->enabled;
}

void GuiHelper::SelectNextEntity(Entity* parent, int dir) {
    if (dir == 0) return;
    dir = dir == 0 ? 0 : dir < 0 ? -1 : 1;

    auto children = EntityManager::GetChildren(parent);
    for (auto it = children.begin(); it != children.end(); ++it) {
        // Find the currently selected entity and make it not selected
        Entity* child = *it;
        if (!IsEntitySelected(child)) continue;
        SetGuisSelected(child, false);

        auto it2 = it;
        while ((it2 += dir) != it) {
            // Wrap
            if (it2 == children.end()) {
                it2 = children.begin();
            } else if (it2 < children.begin()) {
                it2 = children.end() - 1;
            }

            // Make sure it is enabled
            Entity* child2 = *it2;
            if (!IsEntityEnabled(child2)) continue;

            // Select it and finish
            SetGuisSelected(child2, true);
            break;
        }

        break;
    }
}

void GuiHelper::SelectNextEntity(Entity* parent) {
    SelectNextEntity(parent, 1);
}

void GuiHelper::SelectNextEntity(std::string parentTag, int playerIndex) {
    SelectNextEntity(EntityManager::FindEntities(parentTag)[playerIndex]);
}

void GuiHelper::SelectPreviousEntity(Entity* parent) {
    SelectNextEntity(parent, -1);
}

void GuiHelper::SelectPreviousEntity(std::string parentTag, int playerIndex) {
    SelectPreviousEntity(EntityManager::FindEntities(parentTag)[playerIndex]);
}

void GuiHelper::LoadGuiSceneToCamera(size_t cameraIndex, std::string guiScene) {
    CameraComponent *camera = static_cast<CameraComponent*>(EntityManager::GetComponents(ComponentType_Camera)[cameraIndex]);
    ContentManager::LoadScene(guiScene, camera->GetGuiRoot());
}

Entity* GuiHelper::LoadGuiPrefabToCamera(size_t cameraIndex, std::string guiPrefab) {
    CameraComponent *camera = static_cast<CameraComponent*>(EntityManager::GetComponents(ComponentType_Camera)[cameraIndex]);
    return ContentManager::LoadEntity(guiPrefab, camera->GetGuiRoot());
}

void GuiHelper::SetGuisEnabled(Entity *entity, bool enabled) {
	for (GuiComponent *gui : entity->GetComponents<GuiComponent>()) {
		gui->enabled = enabled;
	}
}

void GuiHelper::SetGuisEnabled(std::string entityTag, bool enabled, int playerIndex) {
	SetGuisEnabled(EntityManager::FindEntities(entityTag)[playerIndex], enabled);
}

void GuiHelper::SetGuisSelected(Entity* entity, bool selected) {
    for (GuiComponent *gui : entity->GetComponents<GuiComponent>()) {
        gui->SetSelected(selected);
    }
}

void GuiHelper::SetGuisSelected(std::string entityTag, bool selected, int playerIndex) {
    SetGuisSelected(EntityManager::FindEntities(entityTag)[playerIndex], selected);
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

void GuiHelper::SetGuiYPositions(Entity* entity, float yPosition) {
    for (GuiComponent* gui : entity->GetComponents<GuiComponent>()) {
        glm::vec3 position = gui->transform.GetLocalPosition();
        gui->transform.SetPosition(glm::vec3(position.x, yPosition, position.z));
    }
}

void GuiHelper::AddGuiPositions(Entity* entity, glm::vec3 offset) {
    for (GuiComponent* gui : entity->GetComponents<GuiComponent>()) {
        gui->transform.Translate(offset);
    }
}

void GuiHelper::AddGuiPositions(std::string entityTag, glm::vec3 offset, int playerIndex) {
    AddGuiPositions(EntityManager::FindEntities(entityTag)[playerIndex], offset);
}

void GuiHelper::OpacityEffect(GuiComponent* gui, Time duration, float opacityMod, Time tweenInTime, Time tweenOutTime) {
    auto effect = gui->GetEffect<::OpacityEffect>();
    if (effect) {
        effect->UpdateStartTime();
    } else {
        effect = new ::OpacityEffect(duration, opacityMod, tweenInTime, tweenOutTime);
        gui->AddEffect(effect);
    }
}

GuiComponent* GuiHelper::GetFirstGui(Entity* entity) {
    return entity->GetComponent<GuiComponent>();
}

GuiComponent* GuiHelper::GetFirstGui(std::string entityTag, int playerIndex) {
    return GetFirstGui(EntityManager::FindEntities(entityTag)[playerIndex]);
}

GuiComponent* GuiHelper::GetSecondGui(Entity* entity) {
    return entity->GetComponents<GuiComponent>()[1];
}

GuiComponent* GuiHelper::GetSecondGui(std::string entityTag, int playerIndex) {
    return GetSecondGui(EntityManager::FindEntities(entityTag)[playerIndex]);
}

GuiComponent* GuiHelper::GetThirdGui(Entity* entity) {
    return entity->GetComponents<GuiComponent>()[2];
}

GuiComponent* GuiHelper::GetThirdGui(std::string entityTag, int playerIndex) {
    return GetThirdGui(EntityManager::FindEntities(entityTag)[playerIndex]);
}

GuiComponent* GuiHelper::GetFourthGui(Entity* entity) {
    return entity->GetComponents<GuiComponent>()[3];
}

GuiComponent* GuiHelper::GetFourthGui(std::string entityTag, int playerIndex) {
    return GetFourthGui(EntityManager::FindEntities(entityTag)[playerIndex]);
}
