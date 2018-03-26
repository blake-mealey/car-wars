#include "RocketLauncherComponent.h"
#include "../../Systems/Effects.h"
#include "../CameraComponent.h"
#include "../GuiComponents/GuiHelper.h"

RocketLauncherComponent::RocketLauncherComponent() : WeaponComponent(500.f) {}

void RocketLauncherComponent::Shoot(glm::vec3 position) {
	turnTurret(position);
	if (StateManager::gameTime.GetSeconds() > nextShotTime.GetSeconds()) {
		//Get Vehicle
		Entity* vehicle = GetEntity();

		//Calculate Next Shooting Time
		nextShotTime = StateManager::gameTime.GetSeconds() + timeBetweenShots.GetSeconds();

		//Play Shooting Sound
		Audio& audioManager = Audio::Instance();
		audioManager.PlayAudio("Content/Sounds/rocket-launch.mp3");

		//Create Missile Entity
		Entity* missile = ContentManager::LoadEntity("Missile.json");
		missile->GetComponent<MissileComponent>()->Initialize(vehicle, position);

        TweenChargeIndicator();
	} else {
	}
}

void RocketLauncherComponent::Charge() {
	return;
}

ComponentType RocketLauncherComponent::GetType() {
	return ComponentType_RocketLauncher;
}

void RocketLauncherComponent::HandleEvent(Event *event) {
	return;
}

void RocketLauncherComponent::RenderDebugGui() {
	WeaponComponent::RenderDebugGui();
}