#include "RocketLauncherComponent.h"

RocketLauncherComponent::RocketLauncherComponent() : WeaponComponent(500.f) {}

void RocketLauncherComponent::Shoot() {
	if (StateManager::gameTime.GetTimeSeconds() > nextShotTime.GetTimeSeconds()) {
		//Get Vehicle
		Entity* vehicle = GetEntity();

		//Calculate Next Shooting Time
		nextShotTime = StateManager::gameTime.GetTimeSeconds() + timeBetweenShots.GetTimeSeconds();
		std::cout << "Rocket Shot" << damage << std::endl;

		//Play Shooting Sound
		Audio& audioManager = Audio::Instance();
		audioManager.PlayAudio("Content/Sounds/rocket_launcher_shot.mp3");

		//Create Missile Entity
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