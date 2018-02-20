#include "RailGunComponent.h"

RailGunComponent::RailGunComponent() {}

void RailGunComponent::Shoot() {
	if (StateManager::gameTime.GetTimeSeconds() >= nextShotTime.GetTimeSeconds()) {
		nextChargeTime = StateManager::gameTime + timeBetweenShots;
		std::cout << "Shoot Rail Gun" << std::endl;
		nextShotTime = StateManager::gameTime + (timeBetweenShots + chargeTime);
	} else if (StateManager::gameTime.GetTimeSeconds() < nextChargeTime.GetTimeSeconds()) {
		std::cout << "Rail Gun on Cooldown" << std::endl;
	} else {
		std::cout << "Still Charging..." << std::endl;
	}
}

void RailGunComponent::Charge() {
	if (StateManager::gameTime.GetTimeSeconds() >= nextChargeTime.GetTimeSeconds()) {
		nextShotTime = StateManager::gameTime + chargeTime;
		std::cout << "Charging" << std::endl;
	} else {
		std::cout << "Rail Gun on Cooldown" << std::endl;
	}
}

ComponentType RailGunComponent::GetType() {
	return ComponentType_RailGun;
}

void RailGunComponent::HandleEvent(Event* event) {
	return;
}

void RailGunComponent::RenderDebugGui() {
	RailGunComponent::RenderDebugGui();
}