#include "RocketLauncherComponent.h"

RocketLauncherComponent::RocketLauncherComponent() : WeaponComponent(500.f) {}

void RocketLauncherComponent::Shoot() {
	if (StateManager::gameTime.GetTimeSeconds() > nextShotTime.GetTimeSeconds()) {
		std::cout << "Rocket Shot, Dealt : " << damage << std::endl;
		nextShotTime = StateManager::gameTime.GetTimeSeconds() + timeBetweenShots.GetTimeSeconds();

		Audio& audioManager = Audio::Instance();
		audioManager.PlayAudio("Content/Sounds/rocket_launcher_shot.mp3");

		Entity* vehicle = GetEntity();

		Entity* missile = ContentManager::LoadEntity("Missile.json");
		missile->GetComponent<MissileComponent>()->Initialize(vehicle);
	} else {
		std::cout << "Between Shots" << std::endl;
	}
}

void RocketLauncherComponent::Charge() {
	Shoot();
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