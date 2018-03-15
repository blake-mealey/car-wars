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
    PlayerData* player = Game::Instance().GetPlayerFromEntity(GetEntity());
    if (player) {
        GuiComponent* gui = GuiHelper::GetFirstGui(EntityManager::FindFirstChild(player->camera->GetGuiRoot(), "ChargeIndicator"));
        Transform& mask = gui->GetMask();
        const Time duration = (nextShotTime - StateManager::gameTime) * 0.5;
        Tween* tweenIn = Effects::Instance().CreateTween<glm::vec3, easing::Sine::easeOut>(
            glm::vec3(134.f, 134.f, 0.f), glm::vec3(134.f, 0.f, 0.f), duration,
            [&mask](glm::vec3& value) { mask.SetScale(value); }
        );
        Tween* tweenOut = Effects::Instance().CreateTween<glm::vec3, easing::Sine::easeIn>(
            glm::vec3(134.f, 0.f, 0.f), glm::vec3(134.f, 134.f, 0.f), duration,
            [&mask](glm::vec3& value) { mask.SetScale(value); }
        );
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