#include "GuiHelper.h"

#include "../../Entities/EntityManager.h"
#include "../../Systems/Content/ContentManager.h"
#include "GuiComponent.h"
#include "../../Systems/Effects.h"

GuiComponent* GuiHelper::GetSelectedGui(Entity* entity) {
    for (GuiComponent* gui : entity->GetComponents<GuiComponent>()) {
        if (gui->IsSelected()) return gui;
    }
    return nullptr;
}

GuiComponent* GuiHelper::GetSelectedGui(std::string entityTag, int playerIndex) {
    return GetSelectedGui(GetGuiEntity(entityTag, playerIndex));
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
    SelectNextGui(GetGuiEntity(entityTag, playerIndex));
}

void GuiHelper::SelectPreviousGui(Entity* entity) {
    SelectNextGui(entity, -1);
}

void GuiHelper::SelectPreviousGui(std::string entityTag, int playerIndex) {
    SelectPreviousGui(GetGuiEntity(entityTag, playerIndex));
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
	SetGuisEnabled(GetGuiEntity(entityTag, playerIndex), enabled);
}

void GuiHelper::SetGuisSelected(Entity* entity, bool selected) {
    for (GuiComponent *gui : entity->GetComponents<GuiComponent>()) {
        gui->SetSelected(selected);
    }
}

void GuiHelper::SetGuisSelected(std::string entityTag, bool selected, int playerIndex) {
    SetGuisSelected(GetGuiEntity(entityTag, playerIndex), selected);
}

void GuiHelper::DestroyGuis(Entity* entity) {
	std::vector<GuiComponent*> guis = entity->GetComponents<GuiComponent>();
	for (GuiComponent* gui : guis) {
		EntityManager::DestroyComponent(gui);
	}
}

void GuiHelper::DestroyGuis(std::string entityTag, int playerIndex) {
	DestroyGuis(GetGuiEntity(entityTag, playerIndex));
}

void GuiHelper::SetGuiTexture(Entity* parent, int guiIndex, Texture* texture) {
    vector<GuiComponent*> guis = parent->GetComponents<GuiComponent>();
    if (guis.size() <= guiIndex) return;
    guis[guiIndex]->SetTexture(texture);
}

void GuiHelper::SetGuiText(Entity* entity, int guiIndex, std::string text) {
    vector<GuiComponent*> guis = entity->GetComponents<GuiComponent>();
    if (guis.size() <= guiIndex) return;
    guis[guiIndex]->SetText(text);
}

void GuiHelper::SetGuiText(std::string entityTag, int guiIndex, std::string text, int playerIndex) {
    SetGuiText(GetGuiEntity(entityTag, playerIndex), guiIndex, text);
}

void GuiHelper::SetFirstGuiText(Entity *entity, std::string text) {
    SetGuiText(entity, 0, text);
}

void GuiHelper::SetFirstGuiText(std::string entityTag, std::string text, int playerIndex) {
	SetFirstGuiText(GetGuiEntity(entityTag, playerIndex), text);
}

void GuiHelper::SetSecondGuiText(Entity* entity, std::string text) {
    SetGuiText(entity, 1, text);
}

void GuiHelper::SetSecondGuiText(std::string entityTag, std::string text, int playerIndex) {
    SetSecondGuiText(GetGuiEntity(entityTag, playerIndex), text);
}

void GuiHelper::SetThirdGuiText(Entity* entity, std::string text) {
    SetGuiText(entity, 2, text);
}

void GuiHelper::SetThirdGuiText(std::string entityTag, std::string text, int playerIndex) {
    SetThirdGuiText(GetGuiEntity(entityTag, playerIndex), text);
}

bool GuiHelper::FirstGuiHasText(Entity *entity, std::string text) {
	GuiComponent *gui = entity->GetComponent<GuiComponent>();
    if (!gui) return false;
    return gui->HasText(text);
}

bool GuiHelper::FirstGuiHasText(std::string entityTag, std::string text, int playerIndex) {
	return FirstGuiHasText(GetGuiEntity(entityTag, playerIndex), text);
}

bool GuiHelper::FirstGuiContainsText(Entity* entity, std::string text) {
    GuiComponent *gui = entity->GetComponent<GuiComponent>();
    if (!gui) return false;
    return gui->ContainsText(text);
}

bool GuiHelper::FirstGuiContainsText(std::string entityTag, std::string text, int playerIndex) {
    return FirstGuiContainsText(GetGuiEntity(entityTag, playerIndex), text);
}

void GuiHelper::SetGuiPositions(Entity* entity, glm::vec3 position) {
    for (GuiComponent* gui : entity->GetComponents<GuiComponent>()) {
        gui->transform.SetPosition(position);
    }
}

void GuiHelper::SetGuiPositions(std::string entityTag, glm::vec3 position, int playerIndex) {
    SetGuiPositions(GetGuiEntity(entityTag, playerIndex), position);
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
    AddGuiPositions(GetGuiEntity(entityTag, playerIndex), offset);
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
    return GetFirstGui(GetGuiEntity(entityTag, playerIndex));
}

GuiComponent* GuiHelper::GetSecondGui(Entity* entity) {
    return entity->GetComponents<GuiComponent>()[1];
}

GuiComponent* GuiHelper::GetSecondGui(std::string entityTag, int playerIndex) {
    return GetSecondGui(GetGuiEntity(entityTag, playerIndex));
}

GuiComponent* GuiHelper::GetThirdGui(Entity* entity) {
    return entity->GetComponents<GuiComponent>()[2];
}

GuiComponent* GuiHelper::GetThirdGui(std::string entityTag, int playerIndex) {
    return GetThirdGui(GetGuiEntity(entityTag, playerIndex));
}

GuiComponent* GuiHelper::GetFourthGui(Entity* entity) {
    return entity->GetComponents<GuiComponent>()[3];
}

GuiComponent* GuiHelper::GetFourthGui(std::string entityTag, int playerIndex) {
    return GetFourthGui(GetGuiEntity(entityTag, playerIndex));
}

std::vector<GuiComponent*> GuiHelper::GetGuisRecursive(Entity* parent, std::unordered_set<GuiComponent*> ignoreList) {
	std::vector<GuiComponent*> guis;
	GetGuisRecursive(parent, guis, ignoreList);
	return guis;
}

Entity* GuiHelper::GetGuiEntity(std::string entityTag, int playerIndex) {
	return EntityManager::FindEntities(entityTag)[playerIndex];
    //Entity* root = static_cast<CameraComponent*>(EntityManager::GetComponents(ComponentType_Camera)[playerIndex])->GetGuiRoot();
    //return EntityManager::FindFirstChild(root, entityTag);
}

void GuiHelper::SetGuiColors(Entity* parent, glm::vec4 color) {
    for (GuiComponent* gui : parent->GetComponents<GuiComponent>()) {
        gui->SetFontColor(color);
        gui->SetSelectedFontColor(color);
        gui->SetTextureColor(color);
    }
}

void GuiHelper::GetGuisRecursive(Entity* parent, std::vector<GuiComponent*>& guis, std::unordered_set<GuiComponent*> ignoreList) {
	std::vector<GuiComponent*> components = parent->GetComponents<GuiComponent>();
    for (GuiComponent* gui : components) {
        if (ignoreList.find(gui) != ignoreList.end()) continue;
        guis.push_back(gui);
    }
	for (Entity* child : EntityManager::GetChildren(parent)) {
		GetGuisRecursive(child, guis);
	}
}

void GuiHelper::SetOpacityRecursive(Entity* parent, const float goalOpacity) {
	std::vector<GuiComponent*> guis = GetGuisRecursive(parent);
    for (GuiComponent* gui : guis) {
        gui->SetFontOpacity(goalOpacity);
        gui->SetTextureOpacity(goalOpacity);
    }
}
