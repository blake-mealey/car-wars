#pragma once

#include <string>
#include <vector>

#include "glm/glm.hpp"
#include "../GuiEffects/OpacityEffect.h"

#include "../Tweens/TTween.h"
#include <unordered_set>

class Time;
class CameraComponent;
class Entity;
class GuiComponent;

class GuiHelper {
public:
    static GuiComponent* GetSelectedGui(Entity* entity);
    static GuiComponent* GetSelectedGui(std::string entityTag, int playerIndex = 0);

    static Entity* GetSelectedEntity(Entity* parent);
    static Entity* GetSelectedEntity(std::string parentTag, int playerIndex = 0);

    static std::string GetSelectedGuiText(Entity *entity);
    static std::string GetSelectedGuiText(std::string entityTag, int playerIndex = 0);

    static void SelectNextGui(Entity* entity, int dir);

    static void SelectNextGui(Entity* entity);
    static void SelectNextGui(std::string entityTag, int playerIndex = 0);

    static void SelectPreviousGui(Entity* entity);
    static void SelectPreviousGui(std::string entityTag, int playerIndex = 0);

    static bool IsEntitySelected(Entity* entity);
    static bool IsEntityEnabled(Entity* entity);

    static void SelectNextEntity(Entity* parent, int dir);

    static void SelectNextEntity(Entity* parent);
    static void SelectNextEntity(std::string parentTag, int playerIndex = 0);

    static void SelectPreviousEntity(Entity* parent);
    static void SelectPreviousEntity(std::string parentTag, int playerIndex = 0);

    static void LoadGuiSceneToCamera(size_t cameraIndex, std::string guiScene);
    static Entity* LoadGuiPrefabToCamera(size_t cameraIndex, std::string guiPrefab);

	static void SetGuisEnabled(Entity *entity, bool enabled);
	static void SetGuisEnabled(std::string entityTag, bool enabled, int playerIndex = 0);

    static void SetGuisSelected(Entity* entity, bool selected);
    static void SetGuisSelected(std::string entityTag, bool selected, int playerIndex = 0);

	static void DestroyGuis(Entity* entity);
	static void DestroyGuis(std::string entityTag, int playerIndex = 0);

    static void SetGuiText(Entity *entity, int guiIndex, std::string text);
    static void SetGuiText(std::string entityTag, int guiIndex, std::string text, int playerIndex = 0);

	static void SetFirstGuiText(Entity *entity, std::string text);
	static void SetFirstGuiText(std::string entityTag, std::string text, int playerIndex = 0);

    static void SetSecondGuiText(Entity *entity, std::string text);
    static void SetSecondGuiText(std::string entityTag, std::string text, int playerIndex = 0);

	static bool FirstGuiHasText(Entity *entity, std::string text);
	static bool FirstGuiHasText(std::string entityTag, std::string text, int playerIndex = 0);

	static bool FirstGuiContainsText(Entity *entity, std::string text);
	static bool FirstGuiContainsText(std::string entityTag, std::string text, int playerIndex = 0);
    
    static void SetGuiPositions(Entity* entity, glm::vec3 position);
    static void SetGuiPositions(std::string entityTag, glm::vec3 position, int playerIndex = 0);
    
    static void SetGuiYPositions(Entity* entity, float yPosition);

    static void AddGuiPositions(Entity* entity, glm::vec3 offset);
    static void AddGuiPositions(std::string entityTag, glm::vec3 offset, int playerIndex = 0);
    
    static void OpacityEffect(GuiComponent* gui, Time duration, float opacityMod, Time tweenInTime = 0.0, Time tweenOutTime = 0.0);
    
    static GuiComponent* GetFirstGui(Entity* entity);
    static GuiComponent* GetFirstGui(std::string entityTag, int playerIndex = 0);
    static GuiComponent* GetSecondGui(Entity* entity);
    static GuiComponent* GetSecondGui(std::string entityTag, int playerIndex = 0);
    static GuiComponent* GetThirdGui(Entity* entity);
    static GuiComponent* GetThirdGui(std::string entityTag, int playerIndex = 0);
    static GuiComponent* GetFourthGui(Entity* entity);
    static GuiComponent* GetFourthGui(std::string entityTag, int playerIndex = 0);

	template <float Ease(float t, float b, float c, float d)>
	static TTween<float, Ease>* TweenOpacityRecursive(Entity* parent, const float goalOpacity, const Time duration, Time& clock, std::unordered_set<GuiComponent*> ignoreList = {}) {
		std::vector<GuiComponent*> guis = GetGuisRecursive(parent, ignoreList);

		std::vector<glm::vec2> starts;
		for (GuiComponent* gui : guis) {
			starts.push_back(glm::vec2(gui->GetFontOpacity(), gui->GetTextureOpacity()));
		}

		auto tween = Effects::Instance().CreateTween<float, Ease>(0.f, 1.f, duration, clock);
		tween->SetUpdateCallback([guis, starts, goalOpacity](float& value) mutable {
			for (size_t i = 0; i < guis.size(); ++i) {
				GuiComponent* gui = guis[i];
				glm::vec2 start = starts[i];
				gui->SetFontOpacity(glm::mix(start.x, goalOpacity, value));
				gui->SetTextureOpacity(glm::mix(start.y, goalOpacity, value));
			}
		});
		return tween;
	}
    
	static void SetOpacityRecursive(Entity* parent, float goalOpacity);

	static std::vector<GuiComponent*> GetGuisRecursive(Entity* parent, std::unordered_set<GuiComponent*> ignoreList = {});

    static Entity* GetGuiEntity(std::string entityTag, int playerIndex = 0);
private:
	static void GetGuisRecursive(Entity* parent, std::vector<GuiComponent*>& guis, std::unordered_set<GuiComponent*> ignoreList = {});
};
