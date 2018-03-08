#include "GuiHelper.h"

#include "../Entities/Entity.h"
#include "../Components/Component.h"
#include "../Components/GuiComponents/GuiComponent.h"
#include "../Entities/EntityManager.h"
#include "Content/ContentManager.h"

GuiComponent* GuiHelper::GetSelectedGui(Entity* entity) {
	for (GuiComponent* gui : *entity->GetComponents<GuiComponent>()) {
		if (gui->IsSelected()) return gui;
	}
	/*for (unsigned short index : entity->GetComponentIDs<GuiComponent>()) {
        GuiComponent* gui = &EntityManager::Components<GuiComponent>()[index];
        if (gui->IsSelected()) return gui;
    }*/
}

GuiComponent* GuiHelper::GetSelectedGui(std::string entityTag) {
    return GetSelectedGui(EntityManager::FindEntities(entityTag)[0]);
}

std::string GuiHelper::GetSelectedGuiText(Entity* entity) {
    return GetSelectedGui(entity)->GetText();
}

std::string GuiHelper::GetSelectedGuiText(std::string entityTag) {
    return GetSelectedGui(entityTag)->GetText();
}

void GuiHelper::SelectNextGui(Entity* entity) {
	const vector<unsigned short>& compids = *entity->GetComponentIDs<GuiComponent>();
	for (auto it = compids.begin(); it != compids.end(); ++it) {
		GuiComponent& gui = EntityManager::Components<GuiComponent>()[*it];
		if (gui.IsSelected()) {
			gui.SetSelected(false);
			if (it + 1 != compids.end()) {
				EntityManager::Components<GuiComponent>()[*(it + 1)].SetSelected(true);
			}
			else {
				EntityManager::Components<GuiComponent>()[compids.front()].SetSelected(true);
			}
			break;
		}
	}
    /*for (auto it = entity->GetGuiComponents().begin(); it != entity->GetGuiComponents().end(); ++it) {
        GuiComponent& gui = EntityManager::Components<GuiComponent>()[*it];
        if (gui.IsSelected()) {
            gui.SetSelected(false);
            if (it + 1 != entity->GetGuiComponents().end()) {
				EntityManager::Components<GuiComponent>()[*it+1].SetSelected(true);
            } else {
                EntityManager::Components<GuiComponent>()[entity->GetGuiComponents().front()].SetSelected(true);
            }
            break;
        }
    }*/
}

void GuiHelper::SelectNextGui(std::string entityTag) {
    return SelectNextGui(EntityManager::FindEntities(entityTag)[0]);
}

void GuiHelper::SelectPreviousGui(Entity* entity) {
	const vector<unsigned short>& compids = *entity->GetComponentIDs<GuiComponent>();
    for (auto it = compids.begin(); it != compids.end(); ++it) {
		GuiComponent& gui = EntityManager::Components<GuiComponent>()[*it];
        if (gui.IsSelected()) {
            gui.SetSelected(false);
            if (it - 1 != compids.begin() - 1) {
				EntityManager::Components<GuiComponent>()[*(it - 1)].SetSelected(true);
            } else {
				EntityManager::Components<GuiComponent>()[compids.back()].SetSelected(true);
            }
            break;
        }
    }
}

void GuiHelper::SelectPreviousGui(std::string entityTag) {
    return SelectPreviousGui(EntityManager::FindEntities(entityTag)[0]);
}

void GuiHelper::LoadGuiSceneToCamera(size_t cameraIndex, std::string guiScene) {
    CameraComponent *camera = &EntityManager::Components<CameraComponent>()[cameraIndex];
    ContentManager::LoadScene(guiScene, camera->GetGuiRoot());
}

void GuiHelper::SetGuisEnabled(Entity *entity, bool enabled) {
	//for (Component *component : entity->components) {
		//component->enabled = enabled;
	//}
}

void GuiHelper::SetGuisEnabled(std::string entityTag, bool enabled) {
	SetGuisEnabled(EntityManager::FindEntities(entityTag)[0], enabled);
}

void GuiHelper::SetFirstGuiText(Entity *entity, std::string text) {
	GuiComponent *gui = entity->GetComponent<GuiComponent>();
	gui->SetText(text);
}

void GuiHelper::SetFirstGuiText(std::string entityTag, std::string text) {
	SetFirstGuiText(EntityManager::FindEntities(entityTag)[0], text);
}

bool GuiHelper::FirstGuiTextIs(Entity *entity, std::string text) {
	GuiComponent *gui = entity->GetComponent<GuiComponent>();
	return gui->GetText().compare(text) == 0;
}

bool GuiHelper::FirstGuiTextIs(std::string entityTag, std::string text) {
	return FirstGuiTextIs(EntityManager::FindEntities(entityTag)[0], text);
}