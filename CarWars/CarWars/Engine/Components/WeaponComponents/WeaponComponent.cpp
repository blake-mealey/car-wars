#include "WeaponComponent.h"
#include "../../Systems/Game.h"
#include "../GuiComponents/GuiHelper.h"
#include "../../Entities/EntityManager.h"
#include "../Tweens/Tween.h"
#include "PennerEasing/Sine.h"
#include "../CameraComponent.h"
#include "../../Systems/Effects.h"
#include "../GuiComponents/GuiComponent.h"

WeaponComponent::WeaponComponent(float _damage) : damage(_damage) {}

void WeaponComponent::TweenChargeIndicator() {
    HumanData* player = Game::Instance().GetHumanFromEntity(GetEntity());
    if (player) {
        GuiComponent* gui = GuiHelper::GetFirstGui(EntityManager::FindFirstChild(player->camera->GetGuiRoot(), "ChargeIndicator"));
        Transform& mask = gui->GetMask();
        const Time duration = (nextShotTime - StateManager::gameTime) * 0.5;
        
        auto tweenIn = Effects::Instance().CreateTween<glm::vec3, easing::Sine::easeOut>(
            glm::vec3(134.f, 134.f, 0.f), glm::vec3(134.f, 0.f, 0.f), duration, StateManager::gameTime);
        tweenIn->SetUpdateCallback([&mask](glm::vec3& value) {
			if (StateManager::GetState() != GameState_Playing) return;
			mask.SetScale(value);
		});

        auto tweenOut = Effects::Instance().CreateTween<glm::vec3, easing::Sine::easeIn>(
            glm::vec3(134.f, 0.f, 0.f), glm::vec3(134.f, 134.f, 0.f), duration, StateManager::gameTime);
        tweenOut->SetUpdateCallback([&mask](glm::vec3& value) {
			if (StateManager::GetState() != GameState_Playing) return;
			mask.SetScale(value);
		});
        
        tweenIn->SetNext(tweenOut);
        tweenIn->Start();
    }
}

void WeaponComponent::SetTargetRotation(float _horizontalAngle, float _verticalAngle) {
	targetHorizontalAngle = _horizontalAngle * -1.0f;
	targetVerticalAngle = _verticalAngle * -1.0f;

	horizontalAngle = glm::mix(horizontalAngle, targetHorizontalAngle, 0.05f);
	verticalAngle = glm::mix(verticalAngle, targetVerticalAngle, 0.05f);
}

float WeaponComponent::GetDamage() {
	return damage;
}

ComponentType WeaponComponent::GetType() {
	return ComponentType_Weapons;
}

void WeaponComponent::HandleEvent(Event *event) {
	return;
}

void WeaponComponent::RenderDebugGui() {
    Component::RenderDebugGui();
}