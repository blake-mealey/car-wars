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

	static void SetGuisEnabled(Entity *entity, bool enabled);
	static void SetGuisEnabled(std::string entityTag, bool enabled);

	static void SetFirstGuiText(Entity *entity, std::string text);
	static void SetFirstGuiText(std::string entityTag, std::string text);

	static bool FirstGuiHasText(Entity *entity, std::string text);
	static bool FirstGuiHasText(std::string entityTag, std::string text);

	static bool FirstGuiContainsText(Entity *entity, std::string text);
	static bool FirstGuiContainsText(std::string entityTag, std::string text);
};
