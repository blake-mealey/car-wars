#pragma once
#include <string>

class Entity;
class GuiComponent;

class GuiHelper {
public:
    static GuiComponent* GetSelectedGui(Entity* entity);
    static GuiComponent* GetSelectedGui(std::string entityTag);
    static std::string GetSelectedGuiText(Entity *entity);
    static std::string GetSelectedGuiText(std::string entityTag);

    static void SelectNextGui(Entity* entity);
    static void SelectNextGui(std::string entityTag);

    static void SelectPreviousGui(Entity* entity);
    static void SelectPreviousGui(std::string entityTag);

    static void LoadGuiSceneToCamera(size_t cameraIndex, std::string guiScene);
};
